#pragma once
#include <cstdint>
#include <string>

// Settings structure
struct AppSettings {
    uint32_t background_color;      // CLEAR_COLOR
    uint8_t segment_marking_alpha;  // Alpha value for segment marking (0x00-0xFF)
    char last_game_name[64];        // Name of the last selected game
    
    // Default values
    AppSettings() : 
        background_color(0xdbe2bb),     // Light yellowish (original default)
        segment_marking_alpha(0x05)     // Original alpha value
    {
        last_game_name[0] = '\0';       // Empty string by default
    }
};

// Predefined color presets for easy selection
struct ColorPreset {
    const char* name;
    uint32_t color;
};

static const ColorPreset BACKGROUND_PRESETS[] = {
    {"Light Yellow", 0xdbe2bb},
    {"Light Blue", 0xE0F0FF},
    {"Greyish", 0x98A09C},
    {"White", 0xFFFFFF},
    {"Light Green", 0xE0FFE0},
    {"Cream", 0xFFF8DC}
};

static const int NUM_BG_PRESETS = 6;

// Global settings instance
extern AppSettings g_settings;

// Settings management functions
void load_settings();
void save_settings();
void reset_settings_to_default();

// Last game selection management
void save_last_game(const std::string& game_name);
uint8_t load_last_game_index();
