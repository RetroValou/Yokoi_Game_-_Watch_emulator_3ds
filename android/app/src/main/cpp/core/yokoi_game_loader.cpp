#include "yokoi_game_loader.h"

#include <android/log.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "SM5XX/SM5XX.h"
#include "std/GW_ROM.h"
#include "std/load_file.h"
#include "std/platform_paths.h"
#include "std/settings.h"
#include "std/savestate.h"

// NOTE: Upstream defines get_input_config(...) in this header (non-inline), which is an ODR hazard.
// We include it here with a TU-unique rename so we can use the Virtual_Input type without
// colliding with any other TU that might include the upstream header.
#define get_input_config get_input_config__android_game_loader_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

#include "yokoi_audio.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_layout.h"
#include "yokoi_runtime_state.h"
#include "yokoi_segments_state.h"

namespace {
constexpr const char* kLogTag = "Yokoi";

uint8_t find_game_index_by_ref(const std::string& ref) {
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

void reset_runtime_state_for_new_game() {
    g_emulation_running.store(false);
    g_emulation_paused.store(false);
    g_gamea_pulse_frames.store(0);
    g_gameb_pulse_frames.store(0);
    g_left_action_down.store(false);
    g_right_action_down.store(false);
    g_action_mask.store(0);
    g_start_requested.store(false);
    g_rate_accu = 0;
}
} // namespace

void yokoi_load_game_by_index_and_init(uint8_t idx) {
    std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);

    g_game_index = idx;
    g_game = load_game(idx);
    if (!g_game) {
        __android_log_write(ANDROID_LOG_ERROR, kLogTag, "load_game(index) failed");
        return;
    }

    if (!yokoi_cpu_create_instance(g_cpu, g_game->rom, (uint16_t)g_game->size_rom)) {
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
    yokoi_cpu_set_time_if_needed(g_cpu.get());

    g_input.reset(get_input_config__android_game_loader_tu(g_cpu.get(), g_game->ref));

    g_segments.clear();
    if (g_game->segment && g_game->size_segment > 0) {
        g_segments.assign(g_game->segment, g_game->segment + g_game->size_segment);
    }

    {
        std::lock_guard<std::mutex> snap_lock(g_segment_snapshot_mutex);
        g_segments_meta = std::make_shared<std::vector<Segment>>(g_segments);
        const size_t nseg = g_segments_meta ? g_segments_meta->size() : 0;
        g_seg_on_front = std::make_shared<std::vector<uint8_t>>(nseg, 0);
        g_seg_on_back = std::make_shared<std::vector<uint8_t>>(nseg, 0);
        g_seg_generation.fetch_add(1);
    }

    rebuild_layout_from_game();
    yokoi_audio_reconfigure_from_cpu(g_cpu.get());

    // Warm up so we don't start with a blank frame.
    // Also mimic the 3DS "time set grace" during warmup, because startup code can
    // overwrite / latch the clock digits.
    int warmup_time_grace = kTimeSetGracePeriod;
    for (int i = 0; i < 2000; i++) {
        if (g_cpu->step()) {
            if (warmup_time_grace > 0) {
                warmup_time_grace--;
                g_cpu->time_set(false);
                yokoi_cpu_set_time_if_needed(g_cpu.get());
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
        yokoi_cpu_set_time_if_needed(g_cpu.get());
    }

    // Ensure we display a valid initial frame immediately after load.
    g_cpu->segments_state_are_update = true;
    update_segments_from_cpu(g_cpu.get());
    reset_runtime_state_for_new_game();
    g_time_set_grace_counter = kTimeSetGracePeriod;
    g_texture_generation.fetch_add(1);

    __android_log_print(ANDROID_LOG_INFO, kLogTag, "Loaded game: %s (%s)", g_game->name.c_str(), g_game->ref.c_str());
}

uint8_t yokoi_get_default_game_index_for_android() {
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

int yokoi_get_default_menu_load_choice_for_game(uint8_t idx) {
    return save_state_exists(idx) ? 1 : 0;
}
