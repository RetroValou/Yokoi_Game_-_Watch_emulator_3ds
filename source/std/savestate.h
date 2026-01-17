#pragma once
#include <cstdint>
#include <stdio.h>

class SM5XX;

// Magic number to identify save state files
constexpr uint32_t SAVESTATE_MAGIC = 0x47575341; // "GWSA" (Game & Watch Save)
constexpr uint8_t SAVESTATE_VERSION = 1;

// CPU type identifiers
enum CPUType : uint8_t {
    CPU_TYPE_SM5A = 0,
    CPU_TYPE_SM510 = 1,
    CPU_TYPE_SM511_2 = 2
};

// Common header for all save states
struct SaveStateHeader {
    uint32_t magic;           // Magic number for validation
    uint8_t version;          // Save state format version
    uint8_t game_index;       // Informational only (pack ordering can change); identity is via ref-based filename.
    uint8_t cpu_type;         // Which CPU type (SM5A, SM510, SM511_2)
    uint8_t reserved;         // Padding for alignment
    uint32_t data_size;       // Size of CPU-specific data that follows
};

// Save state management functions
bool save_game_state(SM5XX* cpu, uint8_t game_index);
bool load_game_state(SM5XX* cpu, uint8_t game_index);
bool save_state_exists(uint8_t game_index);
void delete_game_state(uint8_t game_index);

// Get save file path for a game
const char* get_save_path(uint8_t game_index);
