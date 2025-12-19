#include <jni.h>

#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <GLES3/gl3.h>

#include <time.h>

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm>
#include <atomic>

#include "std/GW_ROM.h"
#include "std/platform_paths.h"
#include "std/settings.h"
#include "std/load_file.h"

#include "virtual_i_o/virtual_input.h"

#include "SM5XX/SM5XX.h"
#include "SM5XX/SM5A/SM5A.h"
#include "SM5XX/SM510/SM510.h"
#include "SM5XX/SM511_SM512/SM511_2.h"

namespace {
constexpr const char* kLogTag = "Yokoi";
constexpr bool kRenderConsoleOverlay = true;

// Verified-correct PNG atlas mapping for this project:
// - UV origin behaves like "bottom-left", but V must be flipped for OpenGL sampling.
// - Atlas is mirrored on X and must be flipped horizontally.
constexpr bool kUvFlipX = true;
constexpr bool kUvFlipV = true;

void logi(const char* msg) {
    __android_log_write(ANDROID_LOG_INFO, kLogTag, msg);
}

void logi_str(const std::string& s) {
    __android_log_write(ANDROID_LOG_INFO, kLogTag, s.c_str());
}

int g_width = 0;
int g_height = 0;
bool g_core_inited = false;

GLuint g_program = 0;
GLuint g_vao = 0;
GLuint g_vbo = 0;
GLint g_uColor = -1;

GLint g_uTex = -1;
GLint g_uMul = -1;
GLint g_uScale = -1;

GLuint g_tex_segments = 0;
GLuint g_tex_background = 0;
GLuint g_tex_console = 0;
int g_tex_segments_w = 0;
int g_tex_segments_h = 0;
int g_tex_background_w = 0;
int g_tex_background_h = 0;
int g_tex_console_w = 0;
int g_tex_console_h = 0;

GLuint g_tex_white = 0;

AAssetManager* g_asset_manager = nullptr;

struct RenderVertex {
    float x;
    float y;
    float u;
    float v;
};

std::unique_ptr<SM5XX> g_cpu;
const GW_rom* g_game = nullptr;

std::unique_ptr<Virtual_Input> g_input;

std::vector<Segment> g_segments;
uint16_t g_segment_info[8] = {0};
bool g_double_in_one_screen = false;
uint8_t g_nb_screen = 1;
bool g_split_two_screens_to_panels = false;

bool g_emulation_running = false;
int g_gamea_pulse_frames = 0;
bool g_left_action_down = false;
bool g_right_action_down = false;

std::atomic<int> g_pending_game_delta{0};
std::atomic<bool> g_start_requested{false};
std::atomic<int> g_action_mask{0}; // bit0=left action, bit1=right action
std::atomic<bool> g_texture_reload_requested{false};
uint8_t g_game_index = 0;

// Virtual canvas (in "segment pixels") we map to NDC.
float g_canvas_w = 1.0f;
float g_canvas_h = 1.0f;
float g_screen_off_x[2] = {0.0f, 0.0f};
float g_screen_off_y[2] = {0.0f, 0.0f};

// Combined layout (top screen area + bottom console area) in the same coordinate space.
float g_top_canvas_w = 1.0f;
float g_top_canvas_h = 1.0f;
float g_bottom_canvas_w = 0.0f;
float g_bottom_canvas_h = 0.0f;
float g_combined_canvas_w = 1.0f;
float g_combined_canvas_h = 1.0f;
float g_top_off_x = 0.0f;
float g_bottom_off_x = 0.0f;
float g_bottom_off_y = 0.0f;

constexpr float kTargetFps = 60.0f;
uint32_t g_rate_accu = 0;

// ---------------------------
// Audio (very minimal buzzer)
// ---------------------------
std::mutex g_audio_mutex;
std::vector<int16_t> g_audio_ring;
size_t g_audio_r = 0;
size_t g_audio_w = 0;
int g_audio_sample_rate = 0;
uint16_t g_audio_wait = 0;
bool g_audio_curr_value = false;

static void audio_reset_locked() {
    std::fill(g_audio_ring.begin(), g_audio_ring.end(), 0);
    g_audio_r = 0;
    g_audio_w = 0;
    g_audio_wait = 0;
    g_audio_curr_value = false;
}

static void audio_reconfigure_from_cpu(SM5XX* cpu) {
    if (!cpu) {
        return;
    }
    uint16_t div = cpu->sound_divide_frequency ? (uint16_t)cpu->sound_divide_frequency : (uint16_t)1;
    int rate = (int)(cpu->frequency / (uint32_t)div);
    if (rate <= 0) {
        rate = 32768;
    }

    std::lock_guard<std::mutex> lock(g_audio_mutex);
    g_audio_sample_rate = rate;
    // ~2 seconds of audio in ring buffer.
    g_audio_ring.assign((size_t)g_audio_sample_rate * 2u, 0);
    audio_reset_locked();
}

static void audio_push_sample(int16_t s) {
    if (g_audio_ring.empty()) {
        return;
    }

    g_audio_ring[g_audio_w] = s;
    size_t next_w = g_audio_w + 1;
    if (next_w >= g_audio_ring.size()) {
        next_w = 0;
    }
    // Overwrite oldest if full.
    if (next_w == g_audio_r) {
        size_t next_r = g_audio_r + 1;
        if (next_r >= g_audio_ring.size()) {
            next_r = 0;
        }
        g_audio_r = next_r;
    }
    g_audio_w = next_w;
}

static void audio_update_step(SM5XX* cpu) {
    if (!cpu) {
        return;
    }

    uint16_t div = cpu->sound_divide_frequency ? (uint16_t)cpu->sound_divide_frequency : (uint16_t)1;
    g_audio_wait += 1;
    g_audio_curr_value = g_audio_curr_value || cpu->get_active_sound();
    if (g_audio_wait < div) {
        return;
    }
    g_audio_wait = 0;

    // Square wave amplitude (matches 3DS behavior: on/off sample stream).
    constexpr float kLimit = 0.8f;
    int16_t sample = g_audio_curr_value ? (int16_t)(32767.0f * kLimit) : (int16_t)0;
    g_audio_curr_value = false;

    std::lock_guard<std::mutex> lock(g_audio_mutex);
    audio_push_sample(sample);
}

static GLuint compile_shader(GLenum type, const char* src);
static GLuint link_program(GLuint vs, GLuint fs);

static void init_gl_resources() {
    // If the GL context was lost (rotation/background), object IDs from the old context
    // are not valid anymore. Recreate everything unconditionally.
    g_program = 0;
    g_vao = 0;
    g_vbo = 0;
    g_uTex = -1;
    g_uMul = -1;
    g_uScale = -1;
    g_tex_white = 0;

    const char* vs_src =
        "#version 300 es\n"
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aUv;\n"
        "out vec2 vUv;\n"
        "uniform vec2 uScale;\n"
        "void main() {\n"
        "  vUv = aUv;\n"
        "  gl_Position = vec4(aPos.xy * uScale, 0.0, 1.0);\n"
        "}\n";

    const char* fs_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 vUv;\n"
        "uniform sampler2D uTex;\n"
        "uniform vec4 uMul;\n"
        "out vec4 fragColor;\n"
        "void main() { fragColor = texture(uTex, vUv) * uMul; }\n";

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!vs || !fs) {
        return;
    }
    g_program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!g_program) {
        return;
    }

    g_uTex = glGetUniformLocation(g_program, "uTex");
    g_uMul = glGetUniformLocation(g_program, "uMul");
    g_uScale = glGetUniformLocation(g_program, "uScale");

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)(sizeof(float) * 2));
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Fallback 1x1 white texture.
    glGenTextures(1, &g_tex_white);
    glBindTexture(GL_TEXTURE_2D, g_tex_white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    const uint8_t white_px[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_px);
}

static void set_time_cpu(SM5XX* cpu) {
    if (!cpu || cpu->is_time_set()) {
        return;
    }

    time_t now = time(nullptr);
    tm* t = localtime(&now);
    if (!t) {
        return;
    }

    cpu->set_time((uint8_t)t->tm_hour, (uint8_t)t->tm_min, (uint8_t)t->tm_sec);
    cpu->time_set(true);
}

static bool get_cpu_instance(std::unique_ptr<SM5XX>& out, const uint8_t* rom, uint16_t size_rom) {
    if (!rom || size_rom == 0) {
        return false;
    }

    if (size_rom == 1856) {
        out = std::make_unique<SM5A>();
        return true;
    }
    if (size_rom == 4096) {
        // Heuristic from 3DS main.cpp
        for (int i = 0; i < 16; i++) {
            if (rom[i + 704] != 0x00) {
                out = std::make_unique<SM511_2>();
                return true;
            }
        }
        out = std::make_unique<SM510>();
        return true;
    }
    return false;
}

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        GLsizei len = 0;
        glGetShaderInfoLog(s, (GLsizei)sizeof(log), &len, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "shader compile failed: %s", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static GLuint link_program(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        GLsizei len = 0;
        glGetProgramInfoLog(p, (GLsizei)sizeof(log), &len, log);
        __android_log_print(ANDROID_LOG_ERROR, kLogTag, "program link failed: %s", log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

static void rebuild_layout_from_game() {
    g_canvas_w = 1.0f;
    g_canvas_h = 1.0f;
    g_screen_off_x[0] = g_screen_off_x[1] = 0.0f;
    g_screen_off_y[0] = g_screen_off_y[1] = 0.0f;

    if (!g_game || !g_game->segment_info) {
        return;
    }

    // segment_info layout (from 3DS renderer):
    // - Single-screen games commonly provide 6 entries:
    //   [0]=texW, [1]=texH, [2]=scaleDiv, [3]=flags, [4]=scr0W, [5]=scr0H
    // - Two-screen games provide 8 entries (adds scr1W/scr1H).
    // We MUST not read beyond what's present, so we only read [6]/[7] if we detect 2 screens.

    uint16_t tex_w = g_game->segment_info[0];
    uint16_t tex_h = g_game->segment_info[1];
    uint16_t scale = g_game->segment_info[2] ? g_game->segment_info[2] : 1;
    uint16_t flags = g_game->segment_info[3];
    uint16_t scr0w = g_game->segment_info[4];
    uint16_t scr0h = g_game->segment_info[5];

    g_double_in_one_screen = (flags & 0x02) != 0;

    g_nb_screen = 1;
    for (const auto& s : g_segments) {
        if (s.screen == 1) {
            g_nb_screen = 2;
            break;
        }
    }

    uint16_t scr1w = scr0w;
    uint16_t scr1h = scr0h;
    if (g_nb_screen == 2) {
        scr1w = g_game->segment_info[6];
        scr1h = g_game->segment_info[7];
    }

    // Store the values we may re-use elsewhere.
    g_segment_info[0] = tex_w;
    g_segment_info[1] = tex_h;
    g_segment_info[2] = scale;
    g_segment_info[3] = flags;
    g_segment_info[4] = scr0w;
    g_segment_info[5] = scr0h;
    g_segment_info[6] = scr1w;
    g_segment_info[7] = scr1h;

    float w0 = (float)scr0w / (float)scale;
    float h0 = (float)scr0h / (float)scale;
    float w1 = (float)scr1w / (float)scale;
    float h1 = (float)scr1h / (float)scale;

    if (g_nb_screen == 1) {
        g_canvas_w = w0 > 1.0f ? w0 : 1.0f;
        g_canvas_h = h0 > 1.0f ? h0 : 1.0f;
    } else if (g_double_in_one_screen) {
        g_canvas_w = (w0 + w1) > 1.0f ? (w0 + w1) : 1.0f;
        g_canvas_h = (std::max(h0, h1)) > 1.0f ? (std::max(h0, h1)) : 1.0f;
        g_screen_off_x[0] = 0.0f;
        g_screen_off_x[1] = w0;
        g_screen_off_y[0] = (g_canvas_h - h0) * 0.5f;
        g_screen_off_y[1] = (g_canvas_h - h1) * 0.5f;
    } else {
        // Default: stack vertically.
        g_canvas_w = (std::max(w0, w1)) > 1.0f ? (std::max(w0, w1)) : 1.0f;
        g_canvas_h = (h0 + h1) > 1.0f ? (h0 + h1) : 1.0f;
        g_screen_off_x[0] = (g_canvas_w - w0) * 0.5f;
        g_screen_off_y[0] = 0.0f;
        g_screen_off_x[1] = (g_canvas_w - w1) * 0.5f;
        g_screen_off_y[1] = h0;
    }

    // Android layout policy:
    // - Always render exactly two vertical panels.
    // - Single-screen games: top = game screen, bottom = console.
    // - Two-screen games that are "double_in_one_screen" (left/right): top = both screens combined, bottom = console.
    // - Other two-screen games (clamshell top/bottom): top = screen0, bottom = screen1 (no console panel).
    g_split_two_screens_to_panels = (g_nb_screen == 2) && (!g_double_in_one_screen);

    if (g_split_two_screens_to_panels) {
        // Top panel is screen 0.
        g_top_canvas_w = w0 > 1.0f ? w0 : 1.0f;
        g_top_canvas_h = h0 > 1.0f ? h0 : 1.0f;
        // Bottom panel is screen 1.
        g_bottom_canvas_w = w1 > 1.0f ? w1 : 1.0f;
        g_bottom_canvas_h = h1 > 1.0f ? h1 : 1.0f;

        // For this mode we treat each screen as its own panel origin.
        g_screen_off_x[0] = 0.0f;
        g_screen_off_y[0] = 0.0f;
        g_screen_off_x[1] = 0.0f;
        g_screen_off_y[1] = 0.0f;
    } else {
        // Top panel holds the whole game canvas (1 screen or both combined).
        g_top_canvas_w = g_canvas_w;
        g_top_canvas_h = g_canvas_h;

        // Bottom panel holds the console (if present).
        g_bottom_canvas_w = 0.0f;
        g_bottom_canvas_h = 0.0f;
        if (g_game && g_game->console_info) {
            uint16_t scale2 = g_segment_info[2] ? g_segment_info[2] : 1;
            // console_info: [0]=texW [1]=texH [2]=u [3]=v [4]=w [5]=h
            g_bottom_canvas_w = (float)g_game->console_info[4] / (float)scale2;
            g_bottom_canvas_h = (float)g_game->console_info[5] / (float)scale2;
        }
    }

    g_combined_canvas_w = std::max(g_top_canvas_w, g_bottom_canvas_w);
    if (g_combined_canvas_w < 1.0f) {
        g_combined_canvas_w = 1.0f;
    }
    g_combined_canvas_h = g_top_canvas_h + g_bottom_canvas_h;
    if (g_combined_canvas_h < 1.0f) {
        g_combined_canvas_h = 1.0f;
    }

    g_top_off_x = (g_combined_canvas_w - g_top_canvas_w) * 0.5f;
    g_bottom_off_x = (g_combined_canvas_w - g_bottom_canvas_w) * 0.5f;
    g_bottom_off_y = g_top_canvas_h;
}

static uint8_t find_game_index_by_ref(const std::string& ref) {
    size_t n = get_nb_name();
    if (n > 255) n = 255;
    for (size_t i = 0; i < n; i++) {
        const GW_rom* g = load_game((uint8_t)i);
        if (g && g->ref == ref) {
            return (uint8_t)i;
        }
    }
    return 0;
}

static void apply_action_mask(int mask) {
    bool want_left = (mask & 0x01) != 0;
    bool want_right = (mask & 0x02) != 0;

    if (g_input) {
        if (g_left_action_down != want_left) {
            g_input->set_input(PART_LEFT, BUTTON_ACTION, want_left);
            g_left_action_down = want_left;
        }
        if (g_right_action_down != want_right) {
            g_input->set_input(PART_RIGHT, BUTTON_ACTION, want_right);
            g_right_action_down = want_right;
        }
    } else {
        // No mapping available; clear any prior state.
        g_left_action_down = false;
        g_right_action_down = false;
    }
}

static void reset_runtime_state_for_new_game() {
    g_emulation_running = false;
    g_gamea_pulse_frames = 0;
    g_left_action_down = false;
    g_right_action_down = false;
    g_action_mask.store(0);
    g_start_requested.store(false);
    g_rate_accu = 0;

    // Drop textures until Java reloads the correct atlases.
    g_tex_segments = 0;
    g_tex_background = 0;
    g_tex_console = 0;
    g_tex_segments_w = g_tex_segments_h = 0;
    g_tex_background_w = g_tex_background_h = 0;
    g_tex_console_w = g_tex_console_h = 0;
}

static void load_game_by_index_and_init(uint8_t idx) {
    g_game_index = idx;
    g_game = load_game(idx);
    if (!g_game) {
        __android_log_write(ANDROID_LOG_ERROR, kLogTag, "load_game(index) failed");
        return;
    }

    if (!get_cpu_instance(g_cpu, g_game->rom, (uint16_t)g_game->size_rom)) {
        __android_log_write(ANDROID_LOG_ERROR, kLogTag, "Unsupported ROM size / CPU type");
        return;
    }

    g_cpu->init();
    g_cpu->load_rom(g_game->rom, g_game->size_rom);
    g_cpu->load_rom_melody(g_game->melody, g_game->size_melody);
    g_cpu->load_rom_time_addresses(g_game->ref);

    g_input.reset(get_input_config(g_cpu.get(), g_game->ref));

    g_segments.clear();
    if (g_game->segment && g_game->size_segment > 0) {
        g_segments.assign(g_game->segment, g_game->segment + g_game->size_segment);
    }

    rebuild_layout_from_game();
    set_time_cpu(g_cpu.get());
    audio_reconfigure_from_cpu(g_cpu.get());

    // Warm up so we don't start with a blank frame.
    for (int i = 0; i < 2000; i++) {
        if (g_cpu->step() && g_cpu->segments_state_are_update) {
            g_cpu->segments_state_are_update = false;
        }
    }

    reset_runtime_state_for_new_game();
    g_texture_reload_requested.store(true);

    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Loaded game: %s (%s)", g_game->name.c_str(), g_game->ref.c_str());
}

static void load_game0_and_init_cpu() {
    // Default game for Android: Donkey Kong II.
    // (Ball is index 0; we keep that as the safe fallback.)
    uint8_t idx = find_game_index_by_ref("JR_55");
    load_game_by_index_and_init(idx);
}

static void update_segments_from_cpu(SM5XX* cpu) {
    if (!cpu || !cpu->segments_state_are_update) {
        return;
    }

    // Same blink-protection behavior as 3DS renderer.
    auto protect_blinking = [](Segment& seg, bool new_state) {
        seg.state = seg.state && (new_state || seg.buffer_state);
        seg.state = seg.state || (new_state && seg.buffer_state);
    };

    for (auto& seg : g_segments) {
        bool new_state = cpu->get_segments_state(seg.id[0], seg.id[1], seg.id[2]);
        protect_blinking(seg, new_state);
        seg.buffer_state = seg.state;
        seg.state = new_state;
    }

    cpu->segments_state_are_update = false;
}

static void append_rect_ndc(std::vector<RenderVertex>& out, float x, float y, float w, float h) {
    // x/y in canvas space (top-left origin). Convert to NDC.
    float left = (x / g_canvas_w) * 2.0f - 1.0f;
    float right = ((x + w) / g_canvas_w) * 2.0f - 1.0f;
    float top = 1.0f - (y / g_canvas_h) * 2.0f;
    float bottom = 1.0f - ((y + h) / g_canvas_h) * 2.0f;

    // Two triangles.
    // NOTE: this function is now unused (textured rendering uses append_quad_ndc_uv).
    out.push_back({left, bottom, 0.0f, 0.0f});
    out.push_back({right, bottom, 0.0f, 0.0f});
    out.push_back({right, top, 0.0f, 0.0f});

    out.push_back({right, top, 0.0f, 0.0f});
    out.push_back({left, top, 0.0f, 0.0f});
    out.push_back({left, bottom, 0.0f, 0.0f});
}

static void append_quad_ndc_uv(std::vector<RenderVertex>& out,
                               float x, float y, float w, float h,
                               float u0, float v0, float u1, float v1) {
    float left = (x / g_canvas_w) * 2.0f - 1.0f;
    float right = ((x + w) / g_canvas_w) * 2.0f - 1.0f;
    float top = 1.0f - (y / g_canvas_h) * 2.0f;
    float bottom = 1.0f - ((y + h) / g_canvas_h) * 2.0f;

    // Two triangles.
    out.push_back({left, bottom, u0, v0});
    out.push_back({right, bottom, u1, v0});
    out.push_back({right, top, u1, v1});

    out.push_back({right, top, u1, v1});
    out.push_back({left, top, u0, v1});
    out.push_back({left, bottom, u0, v0});
}

static void append_quad_ndc_uv_canvas(std::vector<RenderVertex>& out,
                                      float canvas_w, float canvas_h,
                                      float x, float y, float w, float h,
                                      float u0, float v0, float u1, float v1) {
    if (canvas_w <= 0.0f) canvas_w = 1.0f;
    if (canvas_h <= 0.0f) canvas_h = 1.0f;

    float left = (x / canvas_w) * 2.0f - 1.0f;
    float right = ((x + w) / canvas_w) * 2.0f - 1.0f;
    float top = 1.0f - (y / canvas_h) * 2.0f;
    float bottom = 1.0f - ((y + h) / canvas_h) * 2.0f;

    out.push_back({left, bottom, u0, v0});
    out.push_back({right, bottom, u1, v0});
    out.push_back({right, top, u1, v1});

    out.push_back({right, top, u1, v1});
    out.push_back({left, top, u0, v1});
    out.push_back({left, bottom, u0, v0});
}

static void calc_uv_rect(float texW, float texH,
                         float u, float v, float w, float h,
                         float& out_u0, float& out_v0, float& out_u1, float& out_v1) {
    if (texW <= 0.0f) texW = 1.0f;
    if (texH <= 0.0f) texH = 1.0f;

    out_u0 = u / texW;
    out_u1 = (u + w) / texW;
    if (kUvFlipX) {
        float tmp = out_u0;
        out_u0 = out_u1;
        out_u1 = tmp;
    }

    // Our vertex builder expects v0 for the bottom edge and v1 for the top edge.
    if (kUvFlipV) {
        out_v0 = 1.0f - (v / texH);
        out_v1 = 1.0f - ((v + h) / texH);
    } else {
        out_v0 = v / texH;
        out_v1 = (v + h) / texH;
    }
}

static std::string t3x_path_to_png_asset(const std::string& p) {
    if (p.empty()) return std::string();
    std::string base = p;
    size_t slash = base.find_last_of('/');
    if (slash != std::string::npos) {
        base = base.substr(slash + 1);
    }
    const std::string t3x = ".t3x";
    if (base.size() >= t3x.size() && base.compare(base.size() - t3x.size(), t3x.size(), t3x) == 0) {
        base.replace(base.size() - t3x.size(), t3x.size(), ".png");
    }
    return base;
}
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetAssetManager(JNIEnv* env, jclass, jobject assetManager) {
    if (!assetManager) {
        g_asset_manager = nullptr;
        return;
    }
    g_asset_manager = AAssetManager_fromJava(env, assetManager);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetStorageRoot(JNIEnv* env, jclass, jstring path) {
    if (!path) return;

    const char* utf = env->GetStringUTFChars(path, nullptr);
    if (!utf) return;

    set_storage_root(std::string(utf));
    env->ReleaseStringUTFChars(path, utf);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeInit(JNIEnv*, jclass) {
    // Always (re)create GL resources on surface creation.
    init_gl_resources();

    if (!g_core_inited) {
        // Settings/saves will land in Context.getFilesDir().
        load_settings();

        logi("Native init OK");
        logi_str("Storage root: " + storage_root());
        logi_str("Games detected: " + std::to_string(get_nb_name()));

        load_game0_and_init_cpu();
        g_core_inited = true;
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetAudioSampleRate(JNIEnv*, jclass) {
    // Prefer configured value; fall back to CPU-derived rate.
    if (g_audio_sample_rate > 0) {
        return (jint)g_audio_sample_rate;
    }
    if (g_cpu) {
        uint16_t div = g_cpu->sound_divide_frequency ? (uint16_t)g_cpu->sound_divide_frequency : (uint16_t)1;
        int rate = (int)(g_cpu->frequency / (uint32_t)div);
        return (jint)(rate > 0 ? rate : 32768);
    }
    return (jint)32768;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeAudioRead(JNIEnv* env, jclass, jshortArray pcm, jint frames) {
    if (!pcm || frames <= 0) {
        return 0;
    }

    jsize len = env->GetArrayLength(pcm);
    if (len < frames) {
        frames = len;
    }

    static thread_local std::vector<jshort> tmp;
    if ((int)tmp.size() < frames) {
        tmp.resize((size_t)frames);
    }

    {
        std::lock_guard<std::mutex> lock(g_audio_mutex);
        for (int i = 0; i < frames; i++) {
            if (!g_audio_ring.empty() && g_audio_r != g_audio_w) {
                tmp[(size_t)i] = (jshort)g_audio_ring[g_audio_r];
                g_audio_r++;
                if (g_audio_r >= g_audio_ring.size()) {
                    g_audio_r = 0;
                }
            } else {
                tmp[(size_t)i] = 0;
            }
        }
    }

    env->SetShortArrayRegion(pcm, 0, frames, tmp.data());
    return frames;
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetTextureAssetNames(JNIEnv* env, jclass) {
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray arr = env->NewObjectArray(3, stringClass, env->NewStringUTF(""));
    if (!g_game) {
        return arr;
    }

    const std::string seg = t3x_path_to_png_asset(g_game->path_segment);
    const std::string bg = t3x_path_to_png_asset(g_game->path_background);
    const std::string cs = t3x_path_to_png_asset(g_game->path_console);

    env->SetObjectArrayElement(arr, 0, env->NewStringUTF(seg.c_str()));
    env->SetObjectArrayElement(arr, 1, env->NewStringUTF(bg.c_str()));
    env->SetObjectArrayElement(arr, 2, env->NewStringUTF(cs.c_str()));
    return arr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetTextures(
        JNIEnv*, jclass,
        jint segmentTex, jint segmentW, jint segmentH,
        jint backgroundTex, jint backgroundW, jint backgroundH,
        jint consoleTex, jint consoleW, jint consoleH) {
    g_tex_segments = (GLuint)segmentTex;
    g_tex_segments_w = (int)segmentW;
    g_tex_segments_h = (int)segmentH;

    g_tex_background = (GLuint)backgroundTex;
    g_tex_background_w = (int)backgroundW;
    g_tex_background_h = (int)backgroundH;

    g_tex_console = (GLuint)consoleTex;
    g_tex_console_w = (int)consoleW;
    g_tex_console_h = (int)consoleH;

    // Prefer actual texture sizes for UV normalization if provided.
    if (g_tex_segments_w > 0 && g_tex_segments_h > 0) {
        g_segment_info[0] = (uint16_t)g_tex_segments_w;
        g_segment_info[1] = (uint16_t)g_tex_segments_h;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeResize(JNIEnv*, jclass, jint width, jint height) {
    g_width = width;
    g_height = height;
    glViewport(0, 0, g_width, g_height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeRender(JNIEnv*, jclass) {
    // Always clear to black so letterbox/pillarbox areas are black.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!g_cpu || !g_program) {
        return;
    }

    // Apply queued game switches on the GL thread (avoids races with the renderer).
    int delta = g_pending_game_delta.exchange(0);
    if (delta != 0) {
        size_t n = get_nb_name();
        if (n > 0) {
            int cur = (int)g_game_index;
            int next = cur + delta;
            while (next < 0) next += (int)n;
            next = next % (int)n;
            load_game_by_index_and_init((uint8_t)next);
        }
    }

    // Start gameplay only when requested (middle tap).
    if (g_start_requested.exchange(false) && !g_emulation_running) {
        g_emulation_running = true;
        g_gamea_pulse_frames = 8; // ~130ms at 60fps
        if (g_input) {
            g_input->set_input(PART_SETUP, BUTTON_GAMEA, true);
        } else {
            g_cpu->input_set(0, 2, true);
        }
    }

    // Apply action presses (used during gameplay).
    apply_action_mask(g_action_mask.load());

    // Release the short GAME A press after a few frames (press length is frame-based,
    // not cycle-based, so it remains stable regardless of CPU step count).
    if (g_gamea_pulse_frames > 0 && g_emulation_running) {
        g_gamea_pulse_frames--;
        if (g_gamea_pulse_frames == 0) {
            if (g_input) {
                g_input->set_input(PART_SETUP, BUTTON_GAMEA, false);
            } else {
                // Fallback for early SM5A titles (Ball/Fire/Vermin) where GAME A is often (0,2).
                g_cpu->input_set(0, 2, false);
            }
        }
    }

    // Run CPU for this frame only after the first touch.
    if (g_emulation_running) {
        g_rate_accu += g_cpu->frequency;
        uint32_t steps = (uint32_t)(g_rate_accu / kTargetFps);
        g_rate_accu -= (uint32_t)(steps * kTargetFps);
        while (steps > 0) {
            if (g_cpu->step()) {
                update_segments_from_cpu(g_cpu.get());
            }
            audio_update_step(g_cpu.get());
            steps--;
        }
    }

    glUseProgram(g_program);
    glBindVertexArray(g_vao);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    // Android Bitmaps are typically premultiplied-alpha.
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    auto draw_vertices = [&](GLuint tex, const std::vector<RenderVertex>& verts) {
        if (tex == 0 || verts.empty()) return;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(g_uTex, 0);
        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(RenderVertex)), verts.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
    };

    auto get_screen_base = [&](uint8_t screen, float& outX, float& outY) {
        if (g_split_two_screens_to_panels) {
            if (screen == 0) {
                outX = g_top_off_x;
                outY = 0.0f;
            } else {
                outX = g_bottom_off_x;
                outY = g_bottom_off_y;
            }
        } else {
            outX = g_top_off_x + g_screen_off_x[screen];
            outY = g_screen_off_y[screen];
        }
    };

    // Aspect-correct fit: render the combined (top+bottom) canvas letterboxed into the device.
    float contentW = g_combined_canvas_w > 0.0f ? g_combined_canvas_w : 1.0f;
    float contentH = g_combined_canvas_h > 0.0f ? g_combined_canvas_h : 1.0f;
    float contentAspect = contentW / contentH;
    float viewAspect = 1.0f;
    if (g_width > 0 && g_height > 0) {
        viewAspect = (float)g_width / (float)g_height;
    }

    float sx = 1.0f;
    float sy = 1.0f;
    if (viewAspect > contentAspect) {
        // Wider viewport -> pillarbox.
        sx = contentAspect / viewAspect;
    } else {
        // Taller viewport -> letterbox.
        sy = viewAspect / contentAspect;
    }
    glUniform2f(g_uScale, sx, sy);

    // Fill the top game screen area with the LCD background color.
    // This provides the intended yellow/blue tint regardless of background PNG content.
    uint32_t bg = g_settings.background_color;
    float br = ((bg >> 16) & 0xFF) / 255.0f;
    float bgc = ((bg >> 8) & 0xFF) / 255.0f;
    float bb = (bg & 0xFF) / 255.0f;

    if (g_tex_white != 0) {
        glUniform4f(g_uMul, br, bgc, bb, 1.0f);
        std::vector<RenderVertex> fill_verts;
        if (g_split_two_screens_to_panels) {
            fill_verts.reserve(12);
            // Fill both game panels.
            append_quad_ndc_uv_canvas(fill_verts, contentW, contentH, g_top_off_x, 0.0f, g_top_canvas_w, g_top_canvas_h, 0.0f, 0.0f, 1.0f, 1.0f);
            append_quad_ndc_uv_canvas(fill_verts, contentW, contentH, g_bottom_off_x, g_bottom_off_y, g_bottom_canvas_w, g_bottom_canvas_h, 0.0f, 0.0f, 1.0f, 1.0f);
        } else {
            fill_verts.reserve(6);
            append_quad_ndc_uv_canvas(fill_verts, contentW, contentH, g_top_off_x, 0.0f, g_top_canvas_w, g_top_canvas_h, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        draw_vertices(g_tex_white, fill_verts);
    }

    // Background layer (top).
    if (g_tex_background != 0 && g_game && g_game->background_info && g_tex_background_w > 0 && g_tex_background_h > 0) {
        glUniform4f(g_uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<RenderVertex> bg_verts;
        bg_verts.reserve(g_nb_screen * 6);

        const uint16_t* bi = g_game->background_info;
        float texW = (float)g_tex_background_w;
        float texH = (float)g_tex_background_h;

        for (uint8_t screen = 0; screen < g_nb_screen; screen++) {
            uint16_t id = (uint16_t)(2 + screen * 4);
            float u = (float)bi[id + 0];
            float v = (float)bi[id + 1];
            float w = (float)bi[id + 2];
            float h = (float)bi[id + 3];

            float u0, v0, u1, v1;
            calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);

            uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
            float dw = (float)g_segment_info[4 + screen * 2] / (float)scale;
            float dh = (float)g_segment_info[5 + screen * 2] / (float)scale;
            float dx = 0.0f;
            float dy = 0.0f;
            get_screen_base(screen, dx, dy);

            append_quad_ndc_uv_canvas(bg_verts, contentW, contentH, dx, dy, dw, dh, u0, v0, u1, v1);
        }

        draw_vertices(g_tex_background, bg_verts);
    }

    // Segments layer (top).
    if (g_segment_info[0] > 0 && g_segment_info[1] > 0) {
        std::vector<RenderVertex> seg_verts;
        seg_verts.reserve(g_segments.size() * 6);

        uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float texW = (float)g_segment_info[0];
        float texH = (float)g_segment_info[1];

        for (const auto& seg : g_segments) {
            if (!seg.buffer_state) {
                continue;
            }

            float base_x = 0.0f;
            float base_y = 0.0f;
            get_screen_base(seg.screen, base_x, base_y);

            float sx = (float)seg.pos_scr[0] / (float)scale;
            float sy = (float)seg.pos_scr[1] / (float)scale;
            float sw = (float)seg.size_tex[0] / (float)scale;
            float sh = (float)seg.size_tex[1] / (float)scale;

            sx += base_x;
            sy += base_y;

            float u0 = 0.0f;
            float v0 = 0.0f;
            float u1 = 1.0f;
            float v1 = 1.0f;

            if (g_tex_segments != 0) {
                float u = (float)seg.pos_tex[0];
                float v = (float)seg.pos_tex[1];
                float w = (float)seg.size_tex[0];
                float h = (float)seg.size_tex[1];

                calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);
            }

            append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx, sy, sw, sh, u0, v0, u1, v1);
        }

        // Derive a dark segment tint from the LCD background color.
        // The segment PNGs are treated as an alpha mask; this controls the final "ink" color.
        float seg_r = br * 0.12f;
        float seg_g = bgc * 0.12f;
        float seg_b = bb * 0.12f;

        if (g_tex_segments != 0) {
            glUniform4f(g_uMul, seg_r, seg_g, seg_b, 1.0f);
            draw_vertices(g_tex_segments, seg_verts);
        } else {
            glUniform4f(g_uMul, seg_r, seg_g, seg_b, 1.0f);
            draw_vertices(g_tex_white, seg_verts);
        }
    }

    // Console overlay layer (bottom).
    if (!g_split_two_screens_to_panels && kRenderConsoleOverlay && g_tex_console != 0 && g_game && g_game->console_info && g_tex_console_w > 0 && g_tex_console_h > 0) {
        glUniform4f(g_uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<RenderVertex> cs_verts;
        cs_verts.reserve(6);

        const uint16_t* ci = g_game->console_info;
        float texW = (float)g_tex_console_w;
        float texH = (float)g_tex_console_h;

        float u = (float)ci[2];
        float v = (float)ci[3];
        float w = (float)ci[4];
        float h = (float)ci[5];

        float u0, v0, u1, v1;
        calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);

        uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float dst_w = w / (float)scale;
        float dst_h = h / (float)scale;

        float dx = g_bottom_off_x;
        float dy = g_bottom_off_y;

        // Draw console in the bottom area of the combined canvas (no stretching).
        append_quad_ndc_uv_canvas(cs_verts, contentW, contentH, dx, dy, dst_w, dst_h, u0, v0, u1, v1);
        draw_vertices(g_tex_console, cs_verts);
    }

    glBindVertexArray(0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeTouch(JNIEnv*, jclass, jfloat x, jfloat y, jint action) {
    (void)x;
    (void)y;
    if (!g_cpu) {
        return;
    }

    const float w = (g_width > 0) ? (float)g_width : 1.0f;
    const float xf = x / w;
    const bool left_third = xf < (1.0f / 3.0f);
    const bool right_third = xf > (2.0f / 3.0f);
    const bool left_half = xf < 0.5f;

    switch (action) {
        case 0: // MotionEvent.ACTION_DOWN
        case 5: // MotionEvent.ACTION_POINTER_DOWN
            if (!g_emulation_running) {
                // Pre-game controls: left=previous, middle=start, right=next.
                if (left_third) {
                    g_pending_game_delta.fetch_sub(1);
                } else if (right_third) {
                    g_pending_game_delta.fetch_add(1);
                } else {
                    g_start_requested.store(true);
                }
                // Ensure gameplay buttons aren't stuck.
                g_action_mask.store(0);
            } else {
                // Gameplay controls: touch halves map to left/right action.
                g_action_mask.store(left_half ? 0x01 : 0x02);
            }
            break;

        case 2: // MotionEvent.ACTION_MOVE
            if (g_emulation_running) {
                g_action_mask.store(left_half ? 0x01 : 0x02);
            }
            break;

        case 1: // MotionEvent.ACTION_UP
        case 6: // MotionEvent.ACTION_POINTER_UP
        case 3: // MotionEvent.ACTION_CANCEL
            g_action_mask.store(0);
            break;

        default:
            break;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeConsumeTextureReloadRequest(JNIEnv*, jclass) {
    return g_texture_reload_requested.exchange(false) ? JNI_TRUE : JNI_FALSE;
}
