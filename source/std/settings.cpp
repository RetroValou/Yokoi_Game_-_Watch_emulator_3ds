#include "settings.h"
#include <stdio.h>
#include <string.h>

// Global settings instance
AppSettings g_settings;

// Settings file path on SD card
static const char* SETTINGS_FILE = "sdmc:/3ds/yokoi_gw_settings.dat";

void load_settings() {
    FILE* file = fopen(SETTINGS_FILE, "rb");
    if (file) {
        size_t read = fread(&g_settings, sizeof(AppSettings), 1, file);
        fclose(file);
        
        if (read != 1) {
            // File corrupted, reset to defaults
            reset_settings_to_default();
        }
    } else {
        // No settings file, use defaults
        reset_settings_to_default();
    }
}

void save_settings() {
    FILE* file = fopen(SETTINGS_FILE, "wb");
    if (file) {
        fwrite(&g_settings, sizeof(AppSettings), 1, file);
        fclose(file);
    }
}

void reset_settings_to_default() {
    g_settings = AppSettings(); // Reset to default constructor values
}
