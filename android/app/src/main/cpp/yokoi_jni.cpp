#include <jni.h>

#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

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
#include "std/savestate.h"
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
int g_touch_width = 0;
int g_touch_height = 0;
bool g_core_inited = false;

// Controller mask (Android physical controller) that mirrors the 3DS mapping.
// Bits are defined below; Java sends the full held-state bitmask.
std::atomic<uint32_t> g_controller_mask{0};

enum ControllerBits : uint32_t {
    CTL_DPAD_UP = 1u << 0,
    CTL_DPAD_DOWN = 1u << 1,
    CTL_DPAD_LEFT = 1u << 2,
    CTL_DPAD_RIGHT = 1u << 3,
    CTL_A = 1u << 4,
    CTL_B = 1u << 5,
    CTL_X = 1u << 6,
    CTL_Y = 1u << 7,
    CTL_START = 1u << 8,
    CTL_SELECT = 1u << 9,
    CTL_L1 = 1u << 10,
};

struct GlResources {
    EGLContext ctx = EGL_NO_CONTEXT;
    int width = 0;
    int height = 0;

    GLuint program = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    size_t vbo_capacity_bytes = 0;
    GLint uTex = -1;
    GLint uMul = -1;
    GLint uScale = -1;
    GLuint tex_white = 0;

    GLuint tex_segments = 0;
    GLuint tex_background = 0;
    GLuint tex_console = 0;
    GLuint tex_ui = 0;
    int tex_segments_w = 0;
    int tex_segments_h = 0;
    int tex_background_w = 0;
    int tex_background_h = 0;
    int tex_console_w = 0;
    int tex_console_h = 0;
    int tex_ui_w = 0;
    int tex_ui_h = 0;
};

std::mutex g_gl_mutex;
std::vector<GlResources> g_gl;
std::mutex g_render_mutex;

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
std::atomic<bool> g_emulation_paused{false};
int g_gamea_pulse_frames = 0;
int g_gameb_pulse_frames = 0;
bool g_left_action_down = false;
bool g_right_action_down = false;

enum AppMode : int {
    MODE_MENU_SELECT = 0,
    MODE_MENU_LOAD_PROMPT = 1,
    MODE_GAME = 2,
};

std::atomic<int> g_app_mode{MODE_MENU_SELECT};
std::atomic<int> g_menu_load_choice{0}; // 0=fresh, 1=load savestate

std::atomic<int> g_pending_game_delta{0};
std::atomic<bool> g_start_requested{false};
std::atomic<int> g_action_mask{0}; // bit0=left action, bit1=right action
std::atomic<uint32_t> g_texture_generation{1};
// Which panel renderer is responsible for advancing emulation.
// -1 = combined renderer only, 0 = panel0 renderer, 1 = panel1 renderer.
std::atomic<int> g_emulation_driver_panel{0};
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
    // Keep ring buffer relatively small to avoid building up noticeable latency.
    // We still overwrite on full (dropping oldest) to favor "latest" audio.
    const size_t target = (size_t)g_audio_sample_rate / 2u; // ~0.5s
    g_audio_ring.assign(std::max<size_t>(target, 2048u), 0);
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

static GlResources* get_or_create_gl_for_current_context();

static void init_gl_resources(GlResources& r) {
    // If the GL context was lost (rotation/background), object IDs from the old context
    // are not valid anymore. Recreate everything for this context.
    r.program = 0;
    r.vao = 0;
    r.vbo = 0;
    r.vbo_capacity_bytes = 0;
    r.uTex = -1;
    r.uMul = -1;
    r.uScale = -1;
    r.tex_white = 0;

    r.tex_segments = 0;
    r.tex_background = 0;
    r.tex_console = 0;
    r.tex_segments_w = r.tex_segments_h = 0;
    r.tex_background_w = r.tex_background_h = 0;
    r.tex_console_w = r.tex_console_h = 0;

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
    r.program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!r.program) {
        return;
    }

    r.uTex = glGetUniformLocation(r.program, "uTex");
    r.uMul = glGetUniformLocation(r.program, "uMul");
    r.uScale = glGetUniformLocation(r.program, "uScale");

    glGenVertexArrays(1, &r.vao);
    glGenBuffers(1, &r.vbo);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)(sizeof(float) * 2));
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Fallback 1x1 white texture.
    glGenTextures(1, &r.tex_white);
    glBindTexture(GL_TEXTURE_2D, r.tex_white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    const uint8_t white_px[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_px);
}

static GlResources* get_or_create_gl_for_current_context() {
    EGLContext ctx = eglGetCurrentContext();
    if (ctx == EGL_NO_CONTEXT) {
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(g_gl_mutex);
    for (auto& r : g_gl) {
        if (r.ctx == ctx) {
            return &r;
        }
    }

    GlResources r;
    r.ctx = ctx;
    init_gl_resources(r);
    g_gl.push_back(r);
    return &g_gl.back();
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

static void rebuild_layout_for_menu() {
    // Menu always renders as two stacked panels:
    // - Top: game name/year (provided by Java UI texture)
    // - Bottom: console image
    g_split_two_screens_to_panels = false;

    // RGDS/Android UI canvas: treat each panel as a 640x480 surface.
    g_top_canvas_w = 640.0f;
    g_top_canvas_h = 480.0f;

    // Bottom size based on the selected game's console image.
    g_bottom_canvas_w = 0.0f;
    g_bottom_canvas_h = 0.0f;
    if (g_game && g_game->segment_info) {
        uint16_t scale = g_game->segment_info[2] ? g_game->segment_info[2] : 1;
        g_segment_info[2] = scale;
        if (g_game->console_info) {
            g_bottom_canvas_w = (float)g_game->console_info[4] / (float)scale;
            g_bottom_canvas_h = (float)g_game->console_info[5] / (float)scale;
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

static void menu_select_game_by_index(uint8_t idx) {
    g_game_index = idx;
    g_game = load_game(idx);
    if (!g_game) {
        __android_log_write(ANDROID_LOG_ERROR, kLogTag, "menu_select_game: load_game failed");
        return;
    }
    save_last_game(get_name(idx));
    rebuild_layout_for_menu();
    g_texture_generation.fetch_add(1);
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
        // Only apply touch-based ACTION presses to parts that are configured as an ACTION button.
        if (g_input->left_configuration == CONF_1_BUTTON_ACTION) {
            if (g_left_action_down != want_left) {
                g_input->set_input(PART_LEFT, BUTTON_ACTION, want_left);
                g_left_action_down = want_left;
            }
        }
        if (g_input->right_configuration == CONF_1_BUTTON_ACTION) {
            if (g_right_action_down != want_right) {
                g_input->set_input(PART_RIGHT, BUTTON_ACTION, want_right);
                g_right_action_down = want_right;
            }
        }
    } else {
        // No mapping available; clear any prior state.
        g_left_action_down = false;
        g_right_action_down = false;
    }
}

static void apply_controller_mask(uint32_t ctl_mask, int touch_action_mask) {
    if (!g_input) {
        return;
    }

    const bool d_up = (ctl_mask & CTL_DPAD_UP) != 0;
    const bool d_down = (ctl_mask & CTL_DPAD_DOWN) != 0;
    const bool d_left = (ctl_mask & CTL_DPAD_LEFT) != 0;
    const bool d_right = (ctl_mask & CTL_DPAD_RIGHT) != 0;

    const bool a = (ctl_mask & CTL_A) != 0;
    const bool b = (ctl_mask & CTL_B) != 0;
    const bool x = (ctl_mask & CTL_X) != 0;
    const bool y = (ctl_mask & CTL_Y) != 0;

    const bool start = (ctl_mask & CTL_START) != 0;
    const bool select = (ctl_mask & CTL_SELECT) != 0;
    const bool l1 = (ctl_mask & CTL_L1) != 0;

    const bool touch_left = (touch_action_mask & 0x01) != 0;
    const bool touch_right = (touch_action_mask & 0x02) != 0;

    // Match 3DS mapping from source/main.cpp::input_get()
    // Setup buttons.
    g_input->set_input(PART_SETUP, BUTTON_TIME, l1);
    g_input->set_input(PART_SETUP, BUTTON_GAMEA, start || (g_gamea_pulse_frames > 0));
    g_input->set_input(PART_SETUP, BUTTON_GAMEB, select || (g_gameb_pulse_frames > 0));

    // Left controls.
    if (g_input->left_configuration == CONF_1_BUTTON_ACTION) {
        bool check = false;
        if (g_input->right_configuration == CONF_1_BUTTON_ACTION) {
            check = d_up || d_down || d_left || y;
        } else {
            check = d_up || d_down || d_left || d_right;
        }
        check = check || touch_left;
        g_input->set_input(PART_LEFT, BUTTON_ACTION, check);
    } else {
        g_input->set_input(PART_LEFT, BUTTON_LEFT, d_left);
        g_input->set_input(PART_LEFT, BUTTON_RIGHT, d_right);
        g_input->set_input(PART_LEFT, BUTTON_UP, d_up);
        g_input->set_input(PART_LEFT, BUTTON_DOWN, d_down);
    }

    // Right controls.
    if (g_input->right_configuration == CONF_1_BUTTON_ACTION) {
        bool check = false;
        if (g_input->left_configuration == CONF_1_BUTTON_ACTION) {
            check = a || b || x || d_right;
        } else {
            check = a || b || x || y;
        }
        check = check || touch_right;
        g_input->set_input(PART_RIGHT, BUTTON_ACTION, check);
    } else {
        // Note: 3DS maps right-side directions to face buttons.
        g_input->set_input(PART_RIGHT, BUTTON_LEFT, y);
        g_input->set_input(PART_RIGHT, BUTTON_RIGHT, a);
        g_input->set_input(PART_RIGHT, BUTTON_UP, x);
        g_input->set_input(PART_RIGHT, BUTTON_DOWN, b);
    }
}

static void reset_runtime_state_for_new_game() {
    g_emulation_running = false;
    g_emulation_paused.store(false);
    g_gamea_pulse_frames = 0;
    g_gameb_pulse_frames = 0;
    g_left_action_down = false;
    g_right_action_down = false;
    g_action_mask.store(0);
    g_start_requested.store(false);
    g_rate_accu = 0;
}

static void update_segments_from_cpu(SM5XX* cpu);

// Some games overwrite their clock RAM during early startup.
// The 3DS implementation works around this by re-applying the host time for a short grace period.
static constexpr int kTimeSetGracePeriod = 500;
static int g_time_set_grace_counter = 0;

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

    // Set time early (before warmup). Some titles (notably Donkey Kong I/II) copy the
    // clock digits from internal RAM to display RAM during their startup sequence.
    // If we set time only *after* warmup, they may have already latched the default time.
    g_cpu->time_set(false);
    set_time_cpu(g_cpu.get());

    g_input.reset(get_input_config(g_cpu.get(), g_game->ref));

    g_segments.clear();
    if (g_game->segment && g_game->size_segment > 0) {
        g_segments.assign(g_game->segment, g_game->segment + g_game->size_segment);
    }

    rebuild_layout_from_game();
    audio_reconfigure_from_cpu(g_cpu.get());

    // Warm up so we don't start with a blank frame.
    // Also mimic the 3DS "time set grace" during warmup, because startup code can
    // overwrite / latch the clock digits.
    int warmup_time_grace = kTimeSetGracePeriod;
    for (int i = 0; i < 2000; i++) {
        if (g_cpu->step()) {
            if (warmup_time_grace > 0) {
                warmup_time_grace--;
                g_cpu->time_set(false);
                set_time_cpu(g_cpu.get());
            }
            if (g_cpu->segments_state_are_update) {
                g_cpu->segments_state_are_update = false;
            }
        }
    }

    // Apply the current host time *after* warmup, since some games overwrite the clock RAM
    // during their init routine.
    if (g_cpu) {
        g_cpu->time_set(false);
        set_time_cpu(g_cpu.get());
    }

    // Ensure we display a valid initial frame immediately after load.
    g_cpu->segments_state_are_update = true;
    update_segments_from_cpu(g_cpu.get());

    reset_runtime_state_for_new_game();
    g_time_set_grace_counter = kTimeSetGracePeriod;
    g_texture_generation.fetch_add(1);

    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Loaded game: %s (%s)", g_game->name.c_str(), g_game->ref.c_str());
}

static uint8_t get_default_game_index_for_android() {
    // Prefer last selected game; fall back to Donkey Kong II.
    uint8_t idx = load_last_game_index();
    size_t n = get_nb_name();
    if (n == 0) {
        return 0;
    }
    if (idx >= (uint8_t)n) {
        idx = 0;
    }
    if (idx == 0) {
        idx = find_game_index_by_ref("JR_55");
    }
    return idx;
}

static void start_game_from_menu(bool load_state) {
    const uint8_t idx = g_game_index;
    load_game_by_index_and_init(idx);

    g_app_mode.store(MODE_GAME);
    // 3DS behavior: once loaded, the CPU runs (segments/time work) but the user still has to
    // press GameA/GameB to start a game mode.
    g_emulation_running = true;
    g_emulation_paused.store(false);
    g_gamea_pulse_frames = 0;
    g_gameb_pulse_frames = 0;

    if (load_state) {
        if (!load_game_state(g_cpu.get(), idx)) {
            __android_log_print(ANDROID_LOG_WARN, kLogTag, "No savestate to load for game %u", (unsigned)idx);
        } else {
            if (g_cpu) {
                g_cpu->segments_state_are_update = true;
                update_segments_from_cpu(g_cpu.get());
            }
        }
    }

    // Re-apply time for a short grace period after game start.
    g_time_set_grace_counter = kTimeSetGracePeriod;
}

static void return_to_menu_from_game() {
    // Save progress if we were in a game.
    if (g_app_mode.load() == MODE_GAME && g_cpu) {
        save_game_state(g_cpu.get(), g_game_index);
    }

    g_emulation_running = false;
    g_emulation_paused.store(false);
    g_action_mask.store(0);
    g_start_requested.store(false);
    g_gamea_pulse_frames = 0;
    g_gameb_pulse_frames = 0;

    g_cpu.reset();
    g_input.reset();
    g_segments.clear();

    g_app_mode.store(MODE_MENU_SELECT);
    g_menu_load_choice.store(0);
    menu_select_game_by_index(g_game_index);
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
    // Always ensure GL resources exist for the current GL context.
    (void)get_or_create_gl_for_current_context();

    if (!g_core_inited) {
        // Settings/saves will land in Context.getFilesDir().
        load_settings();

        logi("Native init OK");
        logi_str("Storage root: " + storage_root());
        logi_str("Games detected: " + std::to_string(get_nb_name()));

        // Start in menu mode like the 3DS app.
        g_app_mode.store(MODE_MENU_SELECT);
        g_menu_load_choice.store(0);
        uint8_t idx = get_default_game_index_for_android();
        menu_select_game_by_index(idx);
        g_core_inited = true;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetTouchSurfaceSize(JNIEnv*, jclass, jint width, jint height) {
    g_touch_width = (int)width;
    g_touch_height = (int)height;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetControllerMask(JNIEnv*, jclass, jint mask) {
    g_controller_mask.store((uint32_t)mask);
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
    GlResources* r = get_or_create_gl_for_current_context();
    if (!r) {
        return;
    }

    r->tex_segments = (GLuint)segmentTex;
    r->tex_segments_w = (int)segmentW;
    r->tex_segments_h = (int)segmentH;

    r->tex_background = (GLuint)backgroundTex;
    r->tex_background_w = (int)backgroundW;
    r->tex_background_h = (int)backgroundH;

    r->tex_console = (GLuint)consoleTex;
    r->tex_console_w = (int)consoleW;
    r->tex_console_h = (int)consoleH;

    // Prefer actual texture sizes for UV normalization if provided.
    if (r->tex_segments_w > 0 && r->tex_segments_h > 0) {
        g_segment_info[0] = (uint16_t)r->tex_segments_w;
        g_segment_info[1] = (uint16_t)r->tex_segments_h;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetUiTexture(
        JNIEnv*, jclass,
        jint uiTex, jint uiW, jint uiH) {
    GlResources* r = get_or_create_gl_for_current_context();
    if (!r) {
        return;
    }
    r->tex_ui = (GLuint)uiTex;
    r->tex_ui_w = (int)uiW;
    r->tex_ui_h = (int)uiH;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetAppMode(JNIEnv*, jclass) {
    return (jint)g_app_mode.load();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetMenuLoadChoice(JNIEnv*, jclass) {
    return (jint)g_menu_load_choice.load();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeMenuHasSaveState(JNIEnv*, jclass) {
    return save_state_exists(g_game_index) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetSelectedGameInfo(JNIEnv* env, jclass) {
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray arr = env->NewObjectArray(2, stringClass, env->NewStringUTF(""));
    std::string name = get_name(g_game_index);
    std::string date = get_date(g_game_index);
    env->SetObjectArrayElement(arr, 0, env->NewStringUTF(name.c_str()));
    env->SetObjectArrayElement(arr, 1, env->NewStringUTF(date.c_str()));
    return arr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeAutoSaveState(JNIEnv*, jclass) {
    if (g_app_mode.load() == MODE_GAME && g_cpu) {
        save_game_state(g_cpu.get(), g_game_index);
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetBackgroundColor(JNIEnv*, jclass) {
    return (jint)g_settings.background_color;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetBackgroundColor(JNIEnv*, jclass, jint rgb) {
    // Settings use 0xRRGGBB.
    g_settings.background_color = (uint32_t)rgb & 0x00FFFFFFu;
    save_settings();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetSegmentMarkingAlpha(JNIEnv*, jclass) {
    return (jint)g_settings.segment_marking_alpha;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetSegmentMarkingAlpha(JNIEnv*, jclass, jint alpha) {
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    g_settings.segment_marking_alpha = (uint8_t)alpha;
    save_settings();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeReturnToMenu(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> lock(g_render_mutex);
    return_to_menu_from_game();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetPaused(JNIEnv*, jclass, jboolean paused) {
    g_emulation_paused.store(paused == JNI_TRUE);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeResize(JNIEnv*, jclass, jint width, jint height) {
    GlResources* r = get_or_create_gl_for_current_context();
    if (!r) {
        return;
    }
    r->width = (int)width;
    r->height = (int)height;
    glViewport(0, 0, r->width, r->height);
}

static void render_frame(GlResources& r, int panel) {
    // Always clear to black so letterbox/pillarbox areas are black.
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (r.program == 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_render_mutex);

    const int mode = g_app_mode.load();
    const bool is_menu = (mode != MODE_GAME);

    const bool is_combined = (panel < 0);
    const bool is_panel0 = (panel == 0);
    const bool is_panel1 = (panel == 1);

    const int driver = g_emulation_driver_panel.load();
    const bool is_driver_panel = (driver == 0 && is_panel0) || (driver == 1 && is_panel1);

    // Only the combined renderer or the configured driver panel advances emulation.
    if (is_combined || is_driver_panel) {
        static uint32_t prev_ctl_mask = 0;
        uint32_t ctl_mask = g_controller_mask.load();
        uint32_t ctl_down = ctl_mask & ~prev_ctl_mask;
        prev_ctl_mask = ctl_mask;

        // Apply queued game switches on the GL thread (avoids races with the renderer).
        // In menu mode, this changes the selection. In game mode, ignore.
        int delta = g_pending_game_delta.exchange(0);
        if (delta != 0 && mode == MODE_MENU_SELECT) {
            size_t n = get_nb_name();
            if (n > 0) {
                int cur = (int)g_game_index;
                int next = cur + delta;
                while (next < 0) next += (int)n;
                next = next % (int)n;
                menu_select_game_by_index((uint8_t)next);
            }
        }

        // Controller behavior in menu.
        if (mode == MODE_MENU_SELECT) {
            if (ctl_down & CTL_DPAD_RIGHT) {
                g_pending_game_delta.fetch_add(1);
            }
            if (ctl_down & CTL_DPAD_LEFT) {
                g_pending_game_delta.fetch_sub(1);
            }
            if (ctl_down & CTL_A) {
                g_menu_load_choice.store(0);
                g_app_mode.store(MODE_MENU_LOAD_PROMPT);
            }
        } else if (mode == MODE_MENU_LOAD_PROMPT) {
            if (ctl_down & (CTL_DPAD_LEFT | CTL_DPAD_RIGHT)) {
                int choice = g_menu_load_choice.load();
                choice = (choice == 0) ? 1 : 0;
                // If no savestate exists, force choice to fresh.
                if (choice == 1 && !save_state_exists(g_game_index)) {
                    choice = 0;
                }
                g_menu_load_choice.store(choice);
            }
            if (ctl_down & CTL_B) {
                g_app_mode.store(MODE_MENU_SELECT);
            }
            if (ctl_down & CTL_A) {
                const bool want_load = (g_menu_load_choice.load() != 0) && save_state_exists(g_game_index);
                start_game_from_menu(want_load);
            }
        }

        // Game mode: always run CPU (GameA/GameB are just inputs).
        if (mode == MODE_GAME && g_cpu) {
            int touch_mask = g_action_mask.load();
            if (g_input) {
                apply_controller_mask(ctl_mask, touch_mask);
            } else {
                apply_action_mask(touch_mask);
            }

            // Decrement any short start pulses (apply_controller_mask ORs them in).
            if (g_emulation_running) {
                if (g_gamea_pulse_frames > 0) g_gamea_pulse_frames--;
                if (g_gameb_pulse_frames > 0) g_gameb_pulse_frames--;
            }

            // Run CPU.
            if (g_emulation_running && !g_emulation_paused.load()) {
                g_rate_accu += g_cpu->frequency;
                uint32_t steps = (uint32_t)(g_rate_accu / kTargetFps);
                g_rate_accu -= (uint32_t)(steps * kTargetFps);
                while (steps > 0) {
                    if (g_cpu->step()) {
                        // Match 3DS behavior: for a short period after game start, repeatedly
                        // re-apply the host time because some games stomp their clock RAM.
                        if (g_time_set_grace_counter > 0) {
                            g_time_set_grace_counter--;
                            g_cpu->time_set(false);
                            set_time_cpu(g_cpu.get());
                        }
                        update_segments_from_cpu(g_cpu.get());
                    }
                    audio_update_step(g_cpu.get());
                    steps--;
                }
            }
        }
    }

    glUseProgram(r.program);
    glBindVertexArray(r.vao);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto draw_vertices = [&](GLuint tex, const std::vector<RenderVertex>& verts) {
        if (tex == 0 || verts.empty()) return;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(r.uTex, 0);
        glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
        const size_t bytes = verts.size() * sizeof(RenderVertex);
        if (bytes > r.vbo_capacity_bytes) {
            glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)bytes, verts.data(), GL_DYNAMIC_DRAW);
            r.vbo_capacity_bytes = bytes;
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)bytes, verts.data());
        }
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());
    };

    // Determine which slice of the combined canvas we are drawing.
    float panel_x = 0.0f;
    float panel_y = 0.0f;
    float panel_w = g_combined_canvas_w;
    float panel_h = g_combined_canvas_h;

    if (!is_combined) {
        if (is_panel0) {
            panel_x = g_top_off_x;
            panel_y = 0.0f;
            panel_w = g_top_canvas_w;
            panel_h = g_top_canvas_h;
        } else if (is_panel1) {
            panel_x = g_bottom_off_x;
            panel_y = g_bottom_off_y;
            panel_w = g_bottom_canvas_w;
            panel_h = g_bottom_canvas_h;
        }
    }

    if (panel_w <= 0.0f) panel_w = 1.0f;
    if (panel_h <= 0.0f) panel_h = 1.0f;

    float contentW = panel_w;
    float contentH = panel_h;
    float contentAspect = contentW / contentH;
    float viewAspect = 1.0f;
    if (r.width > 0 && r.height > 0) {
        viewAspect = (float)r.width / (float)r.height;
    }

    float sx = 1.0f;
    float sy = 1.0f;
    if (viewAspect > contentAspect) {
        sx = contentAspect / viewAspect;
    } else {
        sy = viewAspect / contentAspect;
    }
    glUniform2f(r.uScale, sx, sy);

    auto to_local_x = [&](float gx) { return gx - panel_x; };
    auto to_local_y = [&](float gy) { return gy - panel_y; };

    auto get_screen_base_global = [&](uint8_t screen, float& outX, float& outY) {
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

    uint32_t bg = g_settings.background_color;
    float br = ((bg >> 16) & 0xFF) / 255.0f;
    float bgc = ((bg >> 8) & 0xFF) / 255.0f;
    float bb = (bg & 0xFF) / 255.0f;

    const bool panel_is_game = (!is_menu) && (is_combined || is_panel0 || (g_split_two_screens_to_panels && is_panel1));

    // Menu UI layer (top panel): Java-provided texture.
    if (is_menu && r.tex_ui != 0 && r.tex_ui_w > 0 && r.tex_ui_h > 0) {
        // Draw only on top panel, or on combined canvas in the top region.
        const bool want_ui = is_combined || is_panel0;
        if (want_ui) {
            glUniform4f(r.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
            static thread_local std::vector<RenderVertex> ui_verts;
            ui_verts.clear();
            ui_verts.reserve(6);

            float u0 = 0.0f, u1 = 1.0f;
            float v0 = 0.0f, v1 = 1.0f;
            if (kUvFlipV) {
                v0 = 1.0f;
                v1 = 0.0f;
            }

            float dx = 0.0f;
            float dy = 0.0f;
            float dw = g_top_canvas_w;
            float dh = g_top_canvas_h;
            if (is_combined) {
                dx = to_local_x(g_top_off_x);
                dy = to_local_y(0.0f);
            }
            append_quad_ndc_uv_canvas(ui_verts, contentW, contentH, dx, dy, dw, dh, u0, v0, u1, v1);
            draw_vertices(r.tex_ui, ui_verts);
        }
    }

    if (panel_is_game && r.tex_white != 0) {
        glUniform4f(r.uMul, br, bgc, bb, 1.0f);
        static thread_local std::vector<RenderVertex> fill_verts;
        fill_verts.clear();
        fill_verts.reserve(6);
        append_quad_ndc_uv_canvas(fill_verts, contentW, contentH, 0.0f, 0.0f, contentW, contentH, 0.0f, 0.0f, 1.0f, 1.0f);
        draw_vertices(r.tex_white, fill_verts);
    }

    // Background layer.
    if (panel_is_game && r.tex_background != 0 && g_game && g_game->background_info && r.tex_background_w > 0 && r.tex_background_h > 0) {
        const uint16_t* bi = g_game->background_info;
        // Enabled per-title via background_info, same as 3DS.
        const bool want_bg_shadow = (bi[(size_t)(2 + g_nb_screen * 4)] == 1);

        // 3DS-style background shadow (optional per-title flag in background_info).
        // Draw first, slightly offset and dark with low alpha.
        if (want_bg_shadow) {
            // Faint shadow like the 3DS renderer.
            glUniform4f(r.uMul, 0.0f, 0.0f, 0.0f, (float)0x24 / 255.0f);
            static thread_local std::vector<RenderVertex> bg_shadow_verts;
            bg_shadow_verts.clear();
            bg_shadow_verts.reserve(g_nb_screen * 6);

            float texW = (float)r.tex_background_w;
            float texH = (float)r.tex_background_h;

            for (uint8_t screen = 0; screen < g_nb_screen; screen++) {
                if (!is_combined) {
                    if (!g_split_two_screens_to_panels && is_panel1) {
                        continue;
                    }
                    if (g_split_two_screens_to_panels && (int)screen != panel) {
                        continue;
                    }
                }

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

                float gx = 0.0f;
                float gy = 0.0f;
                get_screen_base_global(screen, gx, gy);
                float dx = to_local_x(gx) + 6.0f;
                float dy = to_local_y(gy) + 6.0f;

                append_quad_ndc_uv_canvas(bg_shadow_verts, contentW, contentH, dx, dy, dw, dh, u0, v0, u1, v1);
            }

            draw_vertices(r.tex_background, bg_shadow_verts);
        }

        // Main background.
        glUniform4f(r.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        static thread_local std::vector<RenderVertex> bg_verts;
        bg_verts.clear();
        bg_verts.reserve(g_nb_screen * 6);
        float texW = (float)r.tex_background_w;
        float texH = (float)r.tex_background_h;

        for (uint8_t screen = 0; screen < g_nb_screen; screen++) {
            if (!is_combined) {
                if (!g_split_two_screens_to_panels && is_panel1) {
                    continue;
                }
                if (g_split_two_screens_to_panels && (int)screen != panel) {
                    continue;
                }
            }

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

            float gx = 0.0f;
            float gy = 0.0f;
            get_screen_base_global(screen, gx, gy);
            float dx = to_local_x(gx);
            float dy = to_local_y(gy);

            append_quad_ndc_uv_canvas(bg_verts, contentW, contentH, dx, dy, dw, dh, u0, v0, u1, v1);
        }

        draw_vertices(r.tex_background, bg_verts);
    }

    // Segments layer.
    if (panel_is_game && g_segment_info[0] > 0 && g_segment_info[1] > 0) {
        const bool is_mask = (g_segment_info[3] & 0x01) != 0;

        static thread_local std::vector<RenderVertex> seg_verts;
        seg_verts.clear();
        seg_verts.reserve(g_segments.size() * 6);

        // 3DS-style segment marking/shadow passes are only for non-mask segment atlases.
        static thread_local std::vector<RenderVertex> seg_mark_verts;
        static thread_local std::vector<RenderVertex> seg_shadow_verts;
        if (!is_mask) {
            seg_mark_verts.clear();
            seg_shadow_verts.clear();
            seg_mark_verts.reserve(g_segments.size() * 6);
            seg_shadow_verts.reserve(g_segments.size() * 6);
        } else {
            seg_mark_verts.clear();
            seg_shadow_verts.clear();
        }

        uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float texW = (float)g_segment_info[0];
        float texH = (float)g_segment_info[1];

        for (const auto& seg : g_segments) {
            if (!is_combined) {
                if (!g_split_two_screens_to_panels && is_panel1) {
                    continue;
                }
                if (g_split_two_screens_to_panels && (int)seg.screen != panel) {
                    continue;
                }
            }

            float base_gx = 0.0f;
            float base_gy = 0.0f;
            get_screen_base_global(seg.screen, base_gx, base_gy);

            float sx2 = (float)seg.pos_scr[0] / (float)scale + base_gx;
            float sy2 = (float)seg.pos_scr[1] / (float)scale + base_gy;
            float sw = (float)seg.size_tex[0] / (float)scale;
            float sh = (float)seg.size_tex[1] / (float)scale;

            float sx_local = to_local_x(sx2);
            float sy_local = to_local_y(sy2);

            float u0 = 0.0f;
            float v0 = 0.0f;
            float u1 = 1.0f;
            float v1 = 1.0f;

            if (r.tex_segments != 0) {
                float u = (float)seg.pos_tex[0];
                float v = (float)seg.pos_tex[1];
                float w = (float)seg.size_tex[0];
                float h = (float)seg.size_tex[1];
                calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);
            }

            if (!is_mask) {
                // Segment marking effect: draw ALL segments very faintly (even if off).
                // Matches 3DS: color ~0x101010 with user-controlled alpha.
                append_quad_ndc_uv_canvas(seg_mark_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);

                // Shadow: draw only lit segments, slightly offset and darker.
                if (seg.buffer_state) {
                    append_quad_ndc_uv_canvas(seg_shadow_verts, contentW, contentH, sx_local + 2.0f, sy_local + 2.0f, sw, sh, u0, v0, u1, v1);
                }
            }

            // Main lit segments.
            if (seg.buffer_state) {
                append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
            }
        }

        float seg_r = br * 0.12f;
        float seg_g = bgc * 0.12f;
        float seg_b = bb * 0.12f;

        GLuint seg_tex = (r.tex_segments != 0) ? r.tex_segments : r.tex_white;

        if (!is_mask) {
            // Pass 1: faint marking across all segments.
            float mark_a = (float)g_settings.segment_marking_alpha / 255.0f;
            if (mark_a > 0.0f && !seg_mark_verts.empty()) {
                float m = 16.0f / 255.0f;
                glUniform4f(r.uMul, m, m, m, mark_a);
                draw_vertices(seg_tex, seg_mark_verts);
            }

            // Pass 2: shadow under lit segments.
            if (!seg_shadow_verts.empty()) {
                float s = 17.0f / 255.0f;
                glUniform4f(r.uMul, s, s, s, (float)0x18 / 255.0f);
                draw_vertices(seg_tex, seg_shadow_verts);
            }
        }

        // Pass 3: main lit segments.
        glUniform4f(r.uMul, seg_r, seg_g, seg_b, 1.0f);
        draw_vertices(seg_tex, seg_verts);
    }

    // Console overlay.
    const bool want_console = (!g_split_two_screens_to_panels) && (is_combined || is_panel1);
    if (want_console && kRenderConsoleOverlay && r.tex_console != 0 && g_game && g_game->console_info && r.tex_console_w > 0 && r.tex_console_h > 0) {
        glUniform4f(r.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        std::vector<RenderVertex> cs_verts;
        cs_verts.reserve(6);

        const uint16_t* ci = g_game->console_info;
        float texW = (float)r.tex_console_w;
        float texH = (float)r.tex_console_h;

        float u = (float)ci[2];
        float v = (float)ci[3];
        float w = (float)ci[4];
        float h = (float)ci[5];

        float u0, v0, u1, v1;
        calc_uv_rect(texW, texH, u, v, w, h, u0, v0, u1, v1);

        uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float dst_w = w / (float)scale;
        float dst_h = h / (float)scale;

        float dx = to_local_x(g_bottom_off_x);
        float dy = to_local_y(g_bottom_off_y);
        append_quad_ndc_uv_canvas(cs_verts, contentW, contentH, dx, dy, dst_w, dst_h, u0, v0, u1, v1);
        draw_vertices(r.tex_console, cs_verts);
    }

    glBindVertexArray(0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetEmulationDriverPanel(JNIEnv*, jclass, jint panel) {
    // Clamp to {-1,0,1}.
    int p = (int)panel;
    if (p < -1) p = -1;
    if (p > 1) p = 1;
    g_emulation_driver_panel.store(p);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeRender(JNIEnv*, jclass) {
    GlResources* r = get_or_create_gl_for_current_context();
    if (!r) {
        return;
    }
    render_frame(*r, -1);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeRenderPanel(JNIEnv*, jclass, jint panel) {
    GlResources* r = get_or_create_gl_for_current_context();
    if (!r) {
        return;
    }
    render_frame(*r, (int)panel);
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeTouch(JNIEnv*, jclass, jfloat x, jfloat y, jint action) {
    const float w = (g_touch_width > 0) ? (float)g_touch_width : 1.0f;
    const float h = (g_touch_height > 0) ? (float)g_touch_height : 1.0f;
    const float xf = x / w;
    const float yf = y / h;
    const bool in_bottom_half = (yf >= 0.5f);
    const bool left_third = xf < (1.0f / 3.0f);
    const bool right_third = xf > (2.0f / 3.0f);
    const bool left_half = xf < 0.5f;
    const bool center_third = (!left_third && !right_third);

    const int mode = g_app_mode.load();

    switch (action) {
        case 0: // MotionEvent.ACTION_DOWN
        case 5: // MotionEvent.ACTION_POINTER_DOWN
            if (mode == MODE_MENU_SELECT) {
                if (!in_bottom_half) {
                    break;
                }
                if (left_third) {
                    g_pending_game_delta.fetch_sub(1);
                } else if (right_third) {
                    g_pending_game_delta.fetch_add(1);
                } else {
                    // Center tap on lower screen: open load prompt.
                    g_menu_load_choice.store(0);
                    g_app_mode.store(MODE_MENU_LOAD_PROMPT);
                }
                break;
            }
            if (mode == MODE_MENU_LOAD_PROMPT) {
                if (!in_bottom_half) {
                    break;
                }
                if (left_half) {
                    start_game_from_menu(false);
                } else {
                    bool has = save_state_exists(g_game_index);
                    start_game_from_menu(has);
                }
                break;
            }
            if (mode == MODE_GAME) {
                // Center tap on the lower screen triggers GAME A; otherwise touch halves map to ACTION.
                if (in_bottom_half && center_third) {
                    g_action_mask.store(0);
                    g_gamea_pulse_frames = 8;
                } else {
                    g_action_mask.store(left_half ? 0x01 : 0x02);
                }
            }
            break;

        case 2: // MotionEvent.ACTION_MOVE
            if (mode == MODE_GAME) {
                g_action_mask.store(left_half ? 0x01 : 0x02);
            }
            break;

        case 1: // MotionEvent.ACTION_UP
        case 6: // MotionEvent.ACTION_POINTER_UP
        case 3: // MotionEvent.ACTION_CANCEL
            if (mode == MODE_GAME) {
                g_action_mask.store(0);
            }
            break;

        default:
            break;
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeConsumeTextureReloadRequest(JNIEnv*, jclass) {
    // Deprecated in favor of nativeGetTextureGeneration(); keep for compatibility.
    static std::atomic<uint32_t> last_seen{0};
    uint32_t cur = g_texture_generation.load();
    uint32_t prev = last_seen.load();
    if (cur != prev) {
        last_seen.store(cur);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetTextureGeneration(JNIEnv*, jclass) {
    return (jint)g_texture_generation.load();
}
