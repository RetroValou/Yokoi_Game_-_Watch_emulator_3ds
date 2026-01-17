#include <jni.h>

#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <chrono>

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <limits>

#include <thread>
#include <condition_variable>

#include "std/GW_ROM.h"
#include "std/gw_pack.h"
#include "std/platform_paths.h"
#include "std/settings.h"
#include "std/savestate.h"
#include "std/load_file.h"

#include "virtual_i_o/virtual_input.h"

#include "yokoi_audio.h"
#include "yokoi_app_modes.h"
#include "yokoi_app_flow.h"
#include "yokoi_game_loader.h"
#include "yokoi_menu_selection.h"
#include "yokoi_controller_state.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_emulation_thread.h"
#include "yokoi_gl.h"
#include "yokoi_input_mapping.h"
#include "yokoi_layout.h"
#include "yokoi_runtime_state.h"
#include "yokoi_segments_state.h"

#include "SM5XX/SM5XX.h"

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

void loge_str(const std::string& s) {
    __android_log_write(ANDROID_LOG_ERROR, kLogTag, s.c_str());
}

static GlResources* get_or_create_gl_for_current_context();

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
    yokoi_gl_init_resources(r);
    g_gl.push_back(r);
    return &g_gl.back();
}


static void menu_select_game_by_index(uint8_t idx) {
    yokoi_menu_select_game_by_index(idx);
}

static uint8_t get_default_game_index_for_android() {
    return yokoi_get_default_game_index_for_android();
}

static int get_default_menu_load_choice_for_game(uint8_t idx) {
    return yokoi_get_default_menu_load_choice_for_game(idx);
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

extern "C" JNIEXPORT jboolean JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeLoadRomPack(JNIEnv* env, jclass, jstring path) {
    if (!path) {
        return JNI_FALSE;
    }

    const char* utf = env->GetStringUTFChars(path, nullptr);
    if (!utf) {
        return JNI_FALSE;
    }

    std::string err;
    const bool ok = gw_pack::load(std::string(utf), &err);
    env->ReleaseStringUTFChars(path, utf);

    if (!ok) {
        loge_str(std::string("ROM pack load failed: ") + err);
        return JNI_FALSE;
    }

    // If the core is already running, refresh menu selection so updated packs take effect
    // immediately (textures/layout/menu list).
    const size_t n = get_nb_name();
    if (g_core_inited && n > 0) {
        if ((size_t)g_game_index >= n) {
            g_game_index = 0;
        }
        g_app_mode.store(MODE_MENU_SELECT);
        g_menu_load_choice.store(0);
        yokoi_menu_select_game_by_index(g_game_index);
    }

    logi_str(std::string("ROM pack loaded (games=") + std::to_string(n) + ")");
    return JNI_TRUE;
}

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetPackFileBytes(JNIEnv* env, jclass, jstring name) {
    if (!name) {
        return nullptr;
    }
    const char* utf = env->GetStringUTFChars(name, nullptr);
    if (!utf) {
        return nullptr;
    }

    const uint8_t* data = nullptr;
    size_t size = 0;
    const bool ok = gw_pack::get_file_bytes(std::string(utf), data, size);
    env->ReleaseStringUTFChars(name, utf);

    if (!ok || !data || size == 0) {
        return nullptr;
    }

    if (size > (size_t)std::numeric_limits<jsize>::max()) {
        return nullptr;
    }
    jbyteArray out = env->NewByteArray((jsize)size);
    if (!out) {
        return nullptr;
    }
    env->SetByteArrayRegion(out, 0, (jsize)size, reinterpret_cast<const jbyte*>(data));
    return out;
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeInit(JNIEnv*, jclass) {
    // Always ensure GL resources exist for the current GL context.
    (void)get_or_create_gl_for_current_context();

    if (!g_core_inited) {
        // Settings/saves will land in Context.getFilesDir().
        load_settings();

        // Attempt to load an external ROM pack from the default location.
        // Only do this if one isn't already loaded (e.g. via nativeLoadRomPack()).
        if (!gw_pack::is_loaded()) {
            std::string err;
            const std::string p = storage_path("yokoi_pack_rgds.ykp");
            const bool ok = gw_pack::load(p, &err);
            if (!ok) {
                loge_str(std::string("Default ROM pack load failed: ") + err);
            }
        }

        logi("Native init OK");
        logi_str("Storage root: " + storage_root());
        logi_str("Games detected: " + std::to_string(get_nb_name()));

        // Start in menu mode like the 3DS app.
        g_app_mode.store(MODE_MENU_SELECT);
        if (get_nb_name() > 0) {
            g_menu_load_choice.store(get_default_menu_load_choice_for_game(get_default_game_index_for_android()));
            uint8_t idx = get_default_game_index_for_android();
            menu_select_game_by_index(idx);
        }
        g_core_inited = true;
    }

    ensure_emu_thread_started();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeShutdown(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);
    yokoi_aaudio_stop_stream();
    stop_emu_thread();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeStartAaudio(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);
    yokoi_aaudio_start_stream();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeStopAaudio(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);
    yokoi_aaudio_stop_stream();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetTouchSurfaceSize(JNIEnv*, jclass, jint width, jint height) {
    g_touch_width = (int)width;
    g_touch_height = (int)height;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeGetAudioSampleRate(JNIEnv*, jclass) {
    const int rate = yokoi_audio_get_source_rate();
    return (jint)(rate > 0 ? rate : 32768);
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

    yokoi_audio_read(reinterpret_cast<int16_t*>(tmp.data()), frames);

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
    std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);
    if (g_app_mode.load() == MODE_GAME && g_cpu) {
        save_game_state(g_cpu.get(), g_game_index);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeReturnToMenu(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> lock(g_render_mutex);
    yokoi_return_to_menu_from_game();
}

extern "C" JNIEXPORT void JNICALL
Java_com_retrovalou_yokoi_MainActivity_nativeSetPaused(JNIEnv*, jclass, jboolean paused) {
    g_emulation_paused.store(paused == JNI_TRUE);
    notify_emu_thread();
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

    // Only the combined renderer or the configured driver panel handles menu input.
    if (is_combined || is_driver_panel) {
        static uint32_t prev_ctl_mask = 0;
        uint32_t ctl_mask = yokoi_controller_get_mask();
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
                g_menu_load_choice.store(get_default_menu_load_choice_for_game(g_game_index));
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
                yokoi_start_game_from_menu(want_load);
            }
        }

        // Game mode stepping is handled by the dedicated emulation thread.
    }

    glUseProgram(r.program);
    glBindVertexArray(r.vao);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Default shader mode: multiply sampled RGBA by uMul (legacy behavior).
    if (r.uAlphaOnly >= 0) {
        glUniform1f(r.uAlphaOnly, 0.0f);
    }

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

    // In portrait, top-align vertically letterboxed content so the unused space is at the bottom.
    // This helps keep the bottom area clearer for touch controls.
    if (r.uOffset >= 0) {
        float ox = 0.0f;
        float oy = 0.0f;
        if (r.height > r.width && sy < 1.0f) {
            oy = 1.0f - sy;
        }
        glUniform2f(r.uOffset, ox, oy);
    }

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

    // Match 3DS: for mask segment atlases, the "fond" color is the classic dark segment color.
    // (See 3DS: Virtual_Screen::load_visual -> if (is_mask){ curr_fond_color = SEGMENT_COLOR[0]; })
    constexpr uint32_t kMaskFondRgb = 0x080908;
    const bool is_mask_game = (!is_menu && ((g_segment_info[3] & 0x01) != 0));
    uint32_t bg = is_mask_game ? kMaskFondRgb : g_settings.background_color;
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
        if (r.uAlphaOnly >= 0) {
            glUniform1f(r.uAlphaOnly, 0.0f);
        }
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
            if (r.uAlphaOnly >= 0) {
                glUniform1f(r.uAlphaOnly, 0.0f);
            }
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
        if (r.uAlphaOnly >= 0) {
            glUniform1f(r.uAlphaOnly, 0.0f);
        }
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

        // Matches 3DS SEGMENT_COLOR[] palette (see source/virtual_i_o/3ds_screen.h).
        // Used by a couple of "Game & Watch Color" titles where Segment::color_index > 0.
        static constexpr uint32_t kSegmentColorRgb[5] = {
            0x080908u, // classic black segment
            0x9992e7u,
            0x58b9a0u,
            0xff677cu,
            0x3db8e4u,
        };

        static thread_local std::vector<RenderVertex> seg_verts;
        seg_verts.clear();

        // For non-mask segment atlases, some games tag certain segments with a color index.
        // We batch lit segments by color index and draw them as alpha masks with a constant RGB.
        static thread_local std::vector<RenderVertex> seg_color_verts[5];
        bool has_colored_segments = false;
        if (!is_mask) {
            for (auto& v : seg_color_verts) {
                v.clear();
            }
        }

        std::shared_ptr<const std::vector<Segment>> meta;
        std::shared_ptr<std::vector<uint8_t>> on;
        {
            std::lock_guard<std::mutex> snap_lock(g_segment_snapshot_mutex);
            meta = g_segments_meta;
            on = g_seg_on_front;
        }

        const size_t seg_count = meta ? meta->size() : 0;
        seg_verts.reserve(seg_count * 6);
        if (!is_mask) {
            for (auto& v : seg_color_verts) {
                v.reserve(seg_count * 6);
            }
        }

        // 3DS-style segment marking/shadow passes are only for non-mask segment atlases.
        static thread_local std::vector<RenderVertex> seg_mark_verts;
        static thread_local std::vector<RenderVertex> seg_shadow_verts;
        if (!is_mask) {
            seg_mark_verts.clear();
            seg_shadow_verts.clear();
            seg_mark_verts.reserve(seg_count * 6);
            seg_shadow_verts.reserve(seg_count * 6);
        } else {
            seg_mark_verts.clear();
            seg_shadow_verts.clear();
        }

        uint16_t scale = g_segment_info[2] ? g_segment_info[2] : 1;
        float texW = (float)g_segment_info[0];
        float texH = (float)g_segment_info[1];

        if (meta) for (size_t si = 0; si < meta->size(); si++) {
            const auto& seg = (*meta)[si];
            const bool seg_on = (on && si < on->size()) ? ((*on)[si] != 0) : false;
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
                if (seg_on) {
                    append_quad_ndc_uv_canvas(seg_shadow_verts, contentW, contentH, sx_local + 2.0f, sy_local + 2.0f, sw, sh, u0, v0, u1, v1);
                }
            }

            // Main lit segments.
            if (seg_on) {
                if (!is_mask && seg.color_index > 0 && seg.color_index < 5) {
                    has_colored_segments = true;
                    append_quad_ndc_uv_canvas(seg_color_verts[seg.color_index], contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                } else {
                    append_quad_ndc_uv_canvas(seg_verts, contentW, contentH, sx_local, sy_local, sw, sh, u0, v0, u1, v1);
                }
            }
        }

        // Default segment tint uses the current background color (classic LCD look).
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
        if (is_mask) {
            // Match 3DS: mask segment atlases are meant to be drawn with their own RGB.
            // Do NOT apply the LCD tint multiplier (which would turn them "colored").
            if (r.uAlphaOnly >= 0) {
                glUniform1f(r.uAlphaOnly, 0.0f);
            }
            glUniform4f(r.uMul, 1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            if (r.uAlphaOnly >= 0) {
                glUniform1f(r.uAlphaOnly, 0.0f);
            }
            glUniform4f(r.uMul, seg_r, seg_g, seg_b, 1.0f);
        }
        draw_vertices(seg_tex, seg_verts);

        // Extra pass: color-indexed segments (non-mask atlases only).
        // This mirrors the 3DS behavior (Virtual_Screen::update_screen) where the segment color
        // is selected via SEGMENT_COLOR[seg.color_index].
        if (!is_mask && has_colored_segments) {
            if (r.uAlphaOnly >= 0) {
                glUniform1f(r.uAlphaOnly, 1.0f);
            }
            for (uint32_t ci = 1; ci < 5; ci++) {
                if (seg_color_verts[ci].empty()) {
                    continue;
                }
                const uint32_t rgb = kSegmentColorRgb[ci];
                const float cr = (float)((rgb >> 16) & 0xFF) / 255.0f;
                const float cg = (float)((rgb >> 8) & 0xFF) / 255.0f;
                const float cb = (float)(rgb & 0xFF) / 255.0f;
                glUniform4f(r.uMul, cr, cg, cb, 1.0f);
                draw_vertices(seg_tex, seg_color_verts[ci]);
            }
            // Restore default for subsequent layers.
            if (r.uAlphaOnly >= 0) {
                glUniform1f(r.uAlphaOnly, 0.0f);
            }
        }

        // Restore default for subsequent layers.
        if (r.uAlphaOnly >= 0) {
            glUniform1f(r.uAlphaOnly, 0.0f);
        }
    }

    // Console overlay.
    const bool want_console = (!g_split_two_screens_to_panels) && (is_combined || is_panel1);
    if (want_console && kRenderConsoleOverlay && r.tex_console != 0 && g_game && g_game->console_info && r.tex_console_w > 0 && r.tex_console_h > 0) {
        if (r.uAlphaOnly >= 0) {
            glUniform1f(r.uAlphaOnly, 0.0f);
        }
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
                    g_menu_load_choice.store(get_default_menu_load_choice_for_game(g_game_index));
                    g_app_mode.store(MODE_MENU_LOAD_PROMPT);
                }
                break;
            }
            if (mode == MODE_MENU_LOAD_PROMPT) {
                if (!in_bottom_half) {
                    break;
                }
                if (left_half) {
                    {
                        std::lock_guard<std::mutex> lock(g_render_mutex);
                        yokoi_start_game_from_menu(false);
                    }
                } else {
                    bool has = save_state_exists(g_game_index);
                    {
                        std::lock_guard<std::mutex> lock(g_render_mutex);
                        yokoi_start_game_from_menu(has);
                    }
                }
                break;
            }
            if (mode == MODE_GAME) {
                // Center tap on the lower screen triggers GAME A; otherwise touch halves map to ACTION.
                if (in_bottom_half && center_third) {
                    g_action_mask.store(0);
                    g_gamea_pulse_frames.store(8);
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
