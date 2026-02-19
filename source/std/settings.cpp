#include "settings.h"
#include "load_file.h"
#include "platform_paths.h"
#include "debug_log.h"
#include "GW_ROM.h"
#include <stdio.h>
#include <string.h>

// Global settings instance
AppSettings g_settings;

static std::string settings_file_path() {
    return storage_path("yokoi_gw_settings.dat");
}

static std::string last_games_by_mfr_file_path() {
    return storage_path("yokoi_gw_last_game_by_mfr.dat");
}

static constexpr size_t kLastGameRefMax = 16;

// Backwards-compatible layout for when MANUFACTURER_COUNT was 2.
// We keep this so older installs don't lose their last-selected games after adding new manufacturers.
struct LastGamesByManufacturerV1 {
    uint8_t last_manufacturer;
    uint8_t _pad[3];
    char last_game_ref_by_mfr[2][kLastGameRefMax];
};

struct LastGamesByManufacturer {
    // Last selected manufacturer id (used to choose which manufacturer to open on boot).
    uint8_t last_manufacturer;
    uint8_t _pad[3];
    char last_game_ref_by_mfr[GW_rom::MANUFACTURER_COUNT][kLastGameRefMax];
};

static bool try_load_last_games_by_mfr(LastGamesByManufacturer& out) {
    memset(&out, 0, sizeof(out));

    std::string path = last_games_by_mfr_file_path();
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        return false;
    }

    // Read based on on-disk size for forward/backward compatibility.
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    const long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return false;
    }
    (void)fseek(file, 0, SEEK_SET);

    bool ok = false;
    if ((size_t)size == sizeof(LastGamesByManufacturer)) {
        ok = (fread(&out, sizeof(LastGamesByManufacturer), 1, file) == 1);
    } else if ((size_t)size == sizeof(LastGamesByManufacturerV1)) {
        LastGamesByManufacturerV1 v1;
        memset(&v1, 0, sizeof(v1));
        if (fread(&v1, sizeof(v1), 1, file) == 1) {
            out.last_manufacturer = v1.last_manufacturer;
            // Copy known manufacturers.
            // Use memcpy to avoid -Wstringop-truncation warnings; both buffers are fixed-size.
            memcpy(
                out.last_game_ref_by_mfr[GW_rom::MANUFACTURER_NINTENDO],
                v1.last_game_ref_by_mfr[0],
                sizeof(v1.last_game_ref_by_mfr[0])
            );
            out.last_game_ref_by_mfr[GW_rom::MANUFACTURER_NINTENDO][kLastGameRefMax - 1] = '\0';
            memcpy(
                out.last_game_ref_by_mfr[GW_rom::MANUFACTURER_TRONICA],
                v1.last_game_ref_by_mfr[1],
                sizeof(v1.last_game_ref_by_mfr[1])
            );
            out.last_game_ref_by_mfr[GW_rom::MANUFACTURER_TRONICA][kLastGameRefMax - 1] = '\0';
            ok = true;
        }
    }

    fclose(file);

    if (!ok) {
        YOKOI_LOG("settings: last_games_by_mfr unreadable (path='%s'); deleting", path.c_str());
        (void)remove(path.c_str());
        memset(&out, 0, sizeof(out));
        return false;
    }

    if (out.last_manufacturer >= GW_rom::MANUFACTURER_COUNT) {
        YOKOI_LOG(
            "settings: last_games_by_mfr invalid last_mfr=%u (count=%u); resetting",
            (unsigned)out.last_manufacturer,
            (unsigned)GW_rom::MANUFACTURER_COUNT
        );
        out.last_manufacturer = 0;
    }
    return true;
}

static LastGamesByManufacturer load_last_games_by_mfr() {
    LastGamesByManufacturer data;
    (void)try_load_last_games_by_mfr(data);
    return data;
}

static void save_last_games_by_mfr(const LastGamesByManufacturer& data) {
    std::string path = last_games_by_mfr_file_path();
    FILE* file = fopen(path.c_str(), "wb");
    if (!file) {
        YOKOI_LOG("settings: last_games_by_mfr save open failed: '%s'", path.c_str());
        return;
    }
    const size_t wrote = fwrite(&data, sizeof(LastGamesByManufacturer), 1, file);
    fclose(file);
    (void)wrote;
}

void load_settings() {
    std::string path = settings_file_path();
    FILE* file = fopen(path.c_str(), "rb");
    if (file) {
        size_t read = fread(&g_settings, sizeof(AppSettings), 1, file);
        fclose(file);
        
        if (read != 1) {
            // File corrupted, reset to defaults
            YOKOI_LOG("settings: settings.dat corrupted/unreadable (path='%s'), resetting", path.c_str());
            reset_settings_to_default();
        }
    } else {
        // No settings file, use defaults
        reset_settings_to_default();
    }
}

void save_settings() {
    std::string path = settings_file_path();
    FILE* file = fopen(path.c_str(), "wb");
    if (file) {
        const size_t wrote = fwrite(&g_settings, sizeof(AppSettings), 1, file);
        fclose(file);
        (void)wrote;
    } else {
        YOKOI_LOG("settings: settings.dat save open failed: '%s'", path.c_str());
    }
}

void reset_settings_to_default() {
    g_settings = AppSettings(); // Reset to default constructor values
}

uint8_t load_last_selected_manufacturer(uint8_t default_manufacturer) {
    LastGamesByManufacturer data;
    const bool ok = try_load_last_games_by_mfr(data);
    if (!ok) {
        return default_manufacturer;
    }
    return data.last_manufacturer;
}

void save_last_selected_game(uint8_t manufacturer_id, const std::string& game_ref) {
    if (manufacturer_id >= GW_rom::MANUFACTURER_COUNT) {
        return;
    }

    LastGamesByManufacturer data = load_last_games_by_mfr();
    data.last_manufacturer = manufacturer_id;
    strncpy(
        data.last_game_ref_by_mfr[manufacturer_id],
        game_ref.c_str(),
        sizeof(data.last_game_ref_by_mfr[manufacturer_id]) - 1
    );
    data.last_game_ref_by_mfr[manufacturer_id][sizeof(data.last_game_ref_by_mfr[manufacturer_id]) - 1] = '\0';
    save_last_games_by_mfr(data);
}

bool try_load_last_game_index_for_manufacturer(uint8_t manufacturer_id, uint8_t* out_index) {
    if (!out_index) {
        return false;
    }

    if (manufacturer_id >= GW_rom::MANUFACTURER_COUNT) {
        return false;
    }

    const LastGamesByManufacturer data = load_last_games_by_mfr();
    const char* saved_ref = data.last_game_ref_by_mfr[manufacturer_id];
    if (!saved_ref || saved_ref[0] == '\0') {
        return false;
    }

    // Search for the game with matching name.
    const size_t num_games = get_nb_name();
    for (uint8_t i = 0; i < num_games; i++) {
        if (get_ref(i) == saved_ref) {
            *out_index = i;
            return true;
        }
    }

    return false;
}
