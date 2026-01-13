#include "savestate.h"
#include "SM5XX/SM5XX.h"
#include "SM5XX/SM5A/SM5A.h"
#include "SM5XX/SM510/SM510.h"
#include "SM5XX/SM511_SM512/SM511_2.h"
#include "load_file.h"
#include <stdio.h>
#include <string.h>
#include <filesystem>
#include "platform_paths.h"

static std::string savestate_dir() {
    return storage_path("yokoi_gw_saves");
}

// Get CPU type from CPU instance
static CPUType get_cpu_type(SM5XX* cpu) {
    return (CPUType)cpu->get_cpu_type_id();
}

// Get save file path for a game
const char* get_save_path(uint8_t game_index) {
    static char path[256];
    // Use game ref for the savestate filename (stable across pack reorder).
    const GW_rom* game = load_game(game_index);
    if(game) {
        std::string dir = savestate_dir();
        snprintf(path, sizeof(path), "%s/%s.sav", dir.c_str(), game->ref.c_str());
    } else {
        // No game loaded/resolved for this index.
        path[0] = '\0';
    }
    return path;
}

// Ensure save directory exists
static void ensure_save_directory() {
    std::error_code ec;
    std::filesystem::create_directories(savestate_dir(), ec);
}

// Check if a save state file exists for a game
bool save_state_exists(uint8_t game_index) {
    const char* path = get_save_path(game_index);
    if(path[0] == '\0') return false;
    FILE* file = fopen(path, "rb");
    if(!file) return false;
    fclose(file);
    return true;
}

// Save game state to file
bool save_game_state(SM5XX* cpu, uint8_t game_index) {
    if(!cpu) return false;
    
    ensure_save_directory();
    
    const char* path = get_save_path(game_index);
    if(path[0] == '\0') return false;
    FILE* file = fopen(path, "wb");
    if(!file) return false;
    
    // Write header
    SaveStateHeader header;
    header.magic = SAVESTATE_MAGIC;
    header.version = SAVESTATE_VERSION;
    header.game_index = game_index;
    header.cpu_type = get_cpu_type(cpu);
    header.reserved = 0;
    header.data_size = 0;
    
    if(fwrite(&header, sizeof(SaveStateHeader), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Write CPU-specific state
    if(!cpu->save_state(file)) {
        fclose(file);
        return false;
    }
    
    fclose(file);
    return true;
}

// Load game state from file
bool load_game_state(SM5XX* cpu, uint8_t game_index) {
    if(!cpu) return false;
    
    const char* path = get_save_path(game_index);
    if(path[0] == '\0') return false;
    FILE* file = fopen(path, "rb");
    if(!file) return false;
    
    // Read and validate header
    SaveStateHeader header;
    if(fread(&header, sizeof(SaveStateHeader), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Validate magic number
    if(header.magic != SAVESTATE_MAGIC) {
        fclose(file);
        return false;
    }
    
    // Validate version
    if(header.version != SAVESTATE_VERSION) {
        fclose(file);
        return false;
    }
    
    // NOTE: We intentionally do not validate header.game_index. Packs can be reordered,
    // and the savestate path is already keyed by game->ref.
    
    // Validate CPU type
    if(header.cpu_type != get_cpu_type(cpu)) {
        fclose(file);
        return false;
    }
    
    // Load CPU-specific state
    bool success = cpu->load_state(file);
    
    fclose(file);
    return success;
}

// Delete game state file
void delete_game_state(uint8_t game_index) {
    const char* path = get_save_path(game_index);
    if(path[0] == '\0') return;
    remove(path);
}
