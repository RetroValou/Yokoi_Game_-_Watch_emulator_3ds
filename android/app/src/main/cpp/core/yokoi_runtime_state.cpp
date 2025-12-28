#include "yokoi_runtime_state.h"

#include <android/asset_manager.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "SM5XX/SM5XX.h"

// NOTE: virtual_input.h defines a non-inline function `get_input_config(...)` in the header.
// Multiple Android translation units include this header, which causes duplicate-symbol
// link failures. Keep upstream code untouched by renaming the symbol in this TU.
#define get_input_config get_input_config__android_runtime_state_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

int g_width = 0;
int g_height = 0;
int g_touch_width = 0;
int g_touch_height = 0;
bool g_core_inited = false;

std::mutex g_gl_mutex;
std::vector<GlResources> g_gl;
std::mutex g_render_mutex;

std::mutex g_cpu_mutex;

std::thread g_emu_thread;
std::atomic<bool> g_emu_thread_started{false};
std::atomic<bool> g_emu_quit{false};
std::mutex g_emu_sleep_mutex;
std::condition_variable g_emu_cv;

std::mutex g_segment_snapshot_mutex;
std::shared_ptr<const std::vector<Segment>> g_segments_meta;
std::shared_ptr<std::vector<uint8_t>> g_seg_on_front;
std::shared_ptr<std::vector<uint8_t>> g_seg_on_back;
std::atomic<uint32_t> g_seg_generation{1};

void* g_asset_manager = nullptr;

std::unique_ptr<SM5XX> g_cpu;
const GW_rom* g_game = nullptr;
std::unique_ptr<Virtual_Input> g_input;

std::vector<Segment> g_segments;
uint16_t g_segment_info[8] = {0};
bool g_double_in_one_screen = false;
uint8_t g_nb_screen = 1;
bool g_split_two_screens_to_panels = false;

std::atomic<bool> g_emulation_running{false};
std::atomic<bool> g_emulation_paused{false};
std::atomic<int> g_gamea_pulse_frames{0};
std::atomic<int> g_gameb_pulse_frames{0};
std::atomic<bool> g_left_action_down{false};
std::atomic<bool> g_right_action_down{false};

std::atomic<int> g_app_mode{0};
std::atomic<int> g_menu_load_choice{1};

std::atomic<int> g_pending_game_delta{0};
std::atomic<bool> g_start_requested{false};
std::atomic<int> g_action_mask{0};
std::atomic<uint32_t> g_texture_generation{1};
std::atomic<int> g_emulation_driver_panel{0};
uint8_t g_game_index = 0;

float g_canvas_w = 1.0f;
float g_canvas_h = 1.0f;
float g_screen_off_x[2] = {0.0f, 0.0f};
float g_screen_off_y[2] = {0.0f, 0.0f};

float g_top_canvas_w = 1.0f;
float g_top_canvas_h = 1.0f;
float g_bottom_canvas_w = 0.0f;
float g_bottom_canvas_h = 0.0f;
float g_combined_canvas_w = 1.0f;
float g_combined_canvas_h = 1.0f;
float g_top_off_x = 0.0f;
float g_bottom_off_x = 0.0f;
float g_bottom_off_y = 0.0f;

uint32_t g_rate_accu = 0;

int g_time_set_grace_counter = 0;
