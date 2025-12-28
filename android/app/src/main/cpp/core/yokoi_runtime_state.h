#pragma once

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "std/GW_ROM.h"
#include "yokoi_gl.h"

class SM5XX;
class Virtual_Input;
struct GW_rom;

// NOTE: These globals were previously file-static in yokoi_jni.cpp.
// They are Android-only and now shared across Android native modules.

extern int g_width;
extern int g_height;
extern int g_touch_width;
extern int g_touch_height;
extern bool g_core_inited;

extern std::mutex g_gl_mutex;
extern std::vector<GlResources> g_gl;
extern std::mutex g_render_mutex;

extern std::mutex g_cpu_mutex;

extern std::thread g_emu_thread;
extern std::atomic<bool> g_emu_thread_started;
extern std::atomic<bool> g_emu_quit;
extern std::mutex g_emu_sleep_mutex;
extern std::condition_variable g_emu_cv;

extern std::mutex g_segment_snapshot_mutex;
extern std::shared_ptr<const std::vector<Segment>> g_segments_meta;
extern std::shared_ptr<std::vector<uint8_t>> g_seg_on_front;
extern std::shared_ptr<std::vector<uint8_t>> g_seg_on_back;
extern std::atomic<uint32_t> g_seg_generation;

// Asset manager is Android-only; keep as void* here to avoid JNI includes.
extern void* g_asset_manager;

extern std::unique_ptr<SM5XX> g_cpu;
extern const GW_rom* g_game;
extern std::unique_ptr<Virtual_Input> g_input;

extern std::vector<Segment> g_segments;
extern uint16_t g_segment_info[8];
extern bool g_double_in_one_screen;
extern uint8_t g_nb_screen;
extern bool g_split_two_screens_to_panels;

extern std::atomic<bool> g_emulation_running;
extern std::atomic<bool> g_emulation_paused;
extern std::atomic<int> g_gamea_pulse_frames;
extern std::atomic<int> g_gameb_pulse_frames;
extern std::atomic<bool> g_left_action_down;
extern std::atomic<bool> g_right_action_down;

extern std::atomic<int> g_app_mode;
extern std::atomic<int> g_menu_load_choice;

extern std::atomic<int> g_pending_game_delta;
extern std::atomic<bool> g_start_requested;
extern std::atomic<int> g_action_mask;
extern std::atomic<uint32_t> g_texture_generation;
extern std::atomic<int> g_emulation_driver_panel;
extern uint8_t g_game_index;

extern float g_canvas_w;
extern float g_canvas_h;
extern float g_screen_off_x[2];
extern float g_screen_off_y[2];

extern float g_top_canvas_w;
extern float g_top_canvas_h;
extern float g_bottom_canvas_w;
extern float g_bottom_canvas_h;
extern float g_combined_canvas_w;
extern float g_combined_canvas_h;
extern float g_top_off_x;
extern float g_bottom_off_x;
extern float g_bottom_off_y;

extern uint32_t g_rate_accu;

// Some games overwrite their clock RAM during early startup.
// The implementation works around this by re-applying the host time for a short grace period.
constexpr int kTimeSetGracePeriod = 500;
extern int g_time_set_grace_counter;

// Shared constants
constexpr float kTargetFps = 60.0f;

// Helper for modules that need the AAssetManager pointer.
// yokoi_jni.cpp will set this via AAssetManager_fromJava.
inline void yokoi_runtime_set_asset_manager(void* mgr) { g_asset_manager = mgr; }
inline void* yokoi_runtime_get_asset_manager() { return g_asset_manager; }
