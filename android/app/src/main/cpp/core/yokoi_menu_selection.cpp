#include "yokoi_menu_selection.h"

#include <android/log.h>

#include <cstddef>

#include "std/GW_ROM.h"
#include "std/load_file.h"
#include "std/platform_paths.h"
#include "std/settings.h"

#include "yokoi_layout.h"
#include "yokoi_runtime_state.h"

namespace {
constexpr const char* kLogTag = "Yokoi";

bool g_last_by_mfr_init = false;
int16_t g_last_idx_by_mfr[256];

static void init_last_by_mfr_once() {
    if (g_last_by_mfr_init) {
        return;
    }
    for (int i = 0; i < 256; i++) {
        g_last_idx_by_mfr[i] = -1;
    }
    g_last_by_mfr_init = true;
}
}

void yokoi_menu_select_game_by_index(uint8_t idx) {
    init_last_by_mfr_once();

    g_game_index = idx;
    g_game = load_game(idx);
    if (!g_game) {
        __android_log_write(ANDROID_LOG_ERROR, kLogTag, "menu_select_game: load_game failed");
        return;
    }

    // Remember last selected per manufacturer so switching manufacturers restores the user's last choice.
    g_last_idx_by_mfr[g_game->manufacturer] = (int16_t)idx;

    // Persist by stable ref (NOT display name) so we can restore across runs.
    // settings.cpp resolves saved entries by comparing against get_ref(i).
    save_last_selected_game(g_game->manufacturer, get_ref(idx));
    rebuild_layout_for_menu();
    g_texture_generation.fetch_add(1);
}

bool yokoi_menu_try_get_last_index_for_manufacturer(uint8_t manufacturer_id, uint8_t* out_idx) {
    init_last_by_mfr_once();

    if (!out_idx) {
        return false;
    }

    int16_t saved = g_last_idx_by_mfr[manufacturer_id];
    if (saved < 0) {
        // Fresh boot: try persisted value.
        uint8_t persisted = 0;
        if (try_load_last_game_index_for_manufacturer(manufacturer_id, &persisted)) {
            g_last_idx_by_mfr[manufacturer_id] = (int16_t)persisted;
            saved = (int16_t)persisted;
        } else {
            return false;
        }
    }

    const size_t n = get_nb_name();
    if ((size_t)saved >= n) {
        return false;
    }

    const GW_rom* g = load_game((uint8_t)saved);
    if (!g) {
        return false;
    }

    if (g->manufacturer != manufacturer_id) {
        return false;
    }

    *out_idx = (uint8_t)saved;
    return true;
}
