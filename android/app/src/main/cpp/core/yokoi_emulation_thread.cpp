#include "yokoi_emulation_thread.h"

#include <chrono>

#include "SM5XX/SM5XX.h"

#include "yokoi_audio.h"
#include "yokoi_controller_state.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"
#include "yokoi_runtime_state.h"
#include "yokoi_segments_state.h"

namespace {

void emu_thread_main() {
    using clock = std::chrono::steady_clock;
    constexpr auto tick = std::chrono::nanoseconds((long long)(1e9 / 60.0));
    auto next = clock::now();

    while (!g_emu_quit.load()) {
        const bool can_run = (g_app_mode.load() == 2 /* MODE_GAME */) && g_emulation_running.load() && !g_emulation_paused.load();
        if (!can_run) {
            yokoi_audio_set_can_run(false);
            std::unique_lock<std::mutex> lk(g_emu_sleep_mutex);
            g_emu_cv.wait(lk, [] {
                if (g_emu_quit.load()) {
                    return true;
                }
                return (g_app_mode.load() == 2 /* MODE_GAME */) && g_emulation_running.load() && !g_emulation_paused.load();
            });
            next = clock::now();
            continue;
        }

        yokoi_audio_set_can_run(true);

        next += tick;

        {
            std::lock_guard<std::mutex> cpu_lock(g_cpu_mutex);
            if (g_cpu) {
                const uint32_t ctl_mask = yokoi_controller_get_mask();
                const int touch_mask = g_action_mask.load();

                if (g_input) {
                    yokoi_input_apply_controller_mask(g_input.get(), ctl_mask, touch_mask, g_gamea_pulse_frames, g_gameb_pulse_frames);
                } else {
                    yokoi_input_apply_action_mask(g_input.get(), g_left_action_down, g_right_action_down, touch_mask);
                }

                // Decrement any short start pulses (apply_controller_mask ORs them in).
                int a = g_gamea_pulse_frames.load();
                if (a > 0) g_gamea_pulse_frames.store(a - 1);
                int b = g_gameb_pulse_frames.load();
                if (b > 0) g_gameb_pulse_frames.store(b - 1);

                g_rate_accu += g_cpu->frequency;
                uint32_t steps = (uint32_t)(g_rate_accu / kTargetFps);
                g_rate_accu -= (uint32_t)(steps * kTargetFps);
                while (steps > 0) {
                    if (g_cpu->step()) {
                        if (g_time_set_grace_counter > 0) {
                            g_time_set_grace_counter--;
                            g_cpu->time_set(false);
                            yokoi_cpu_set_time_if_needed(g_cpu.get());
                        }
                        update_segments_from_cpu(g_cpu.get());
                    }
                    yokoi_audio_update_step(g_cpu.get());
                    steps--;
                }
            }
        }

        auto now = clock::now();
        if (next > now) {
            std::this_thread::sleep_until(next);
        } else {
            next = now;
        }
    }
}

} // namespace

void notify_emu_thread() {
    g_emu_cv.notify_all();
}

void ensure_emu_thread_started() {
    bool expected = false;
    if (!g_emu_thread_started.compare_exchange_strong(expected, true)) {
        return;
    }
    g_emu_quit.store(false);
    g_emu_thread = std::thread(emu_thread_main);
}

void stop_emu_thread() {
    g_emu_quit.store(true);
    notify_emu_thread();
    if (g_emu_thread.joinable()) {
        g_emu_thread.join();
    }
    g_emu_thread_started.store(false);
}
