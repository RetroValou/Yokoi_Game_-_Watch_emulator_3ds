#include "yokoi_app_flow.h"

#include <android/log.h>

#include <cstdint>
#include <mutex>

#include "std/savestate.h"

// NOTE: Upstream defines get_input_config(...) in this header (non-inline), which is an ODR hazard.
// We include it here with a TU-unique rename so Virtual_Input is a complete type for g_input.reset().
#define get_input_config get_input_config__android_app_flow_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

#include "yokoi_app_modes.h"
#include "yokoi_emulation_thread.h"
#include "yokoi_game_loader.h"
#include "yokoi_menu_selection.h"
#include "yokoi_runtime_state.h"
#include "yokoi_segments_state.h"

namespace {
constexpr const char* kLogTag = "Yokoi";
}

void yokoi_start_game_from_menu(bool load_state) {
    const uint8_t idx = g_game_index;
    yokoi_load_game_by_index_and_init(idx);

    g_app_mode.store(MODE_GAME);
    // 3DS behavior: once loaded, the CPU runs (segments/time work) but the user still has to
    // press GameA/GameB to start a game mode.
    g_emulation_running.store(true);
    g_emulation_paused.store(false);
    g_gamea_pulse_frames.store(0);
    g_gameb_pulse_frames.store(0);

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
    notify_emu_thread();
}

void yokoi_return_to_menu_from_game() {
    std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);

    // Save progress if we were in a game.
    if (g_app_mode.load() == MODE_GAME && g_cpu) {
        save_game_state(g_cpu.get(), g_game_index);
    }

    g_emulation_running.store(false);
    g_emulation_paused.store(false);
    g_action_mask.store(0);
    g_start_requested.store(false);
    g_gamea_pulse_frames.store(0);
    g_gameb_pulse_frames.store(0);

    g_cpu.reset();
    g_input.reset();
    g_segments.clear();

    {
        std::lock_guard<std::mutex> snap_lock(g_segment_snapshot_mutex);
        g_segments_meta = std::make_shared<std::vector<Segment>>();
        g_seg_on_front = std::make_shared<std::vector<uint8_t>>();
        g_seg_on_back = std::make_shared<std::vector<uint8_t>>();
        g_seg_generation.fetch_add(1);
    }

    g_app_mode.store(MODE_MENU_SELECT);
    g_menu_load_choice.store(0);
    yokoi_menu_select_game_by_index(g_game_index);
    notify_emu_thread();
}
