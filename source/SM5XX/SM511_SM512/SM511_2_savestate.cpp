#include "SM511_2.h"
#include "std/timer.h"
#include <stdio.h>
#include <string.h>

// Save SM511_2 CPU state to file
bool SM511_2::save_state(FILE* file) {
    if(!file) return false;
    
    // Save critical CPU state
    fwrite(&program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&s_buffer_program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&r_buffer_program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&ram_address, sizeof(RAMAddress), 1, file);
    fwrite(&accumulator, sizeof(uint8_t), 1, file);
    fwrite(&carry, sizeof(bool), 1, file);
    
    // RAM - holds game state AND display
    fwrite(ram, sizeof(uint8_t), SM511_2_RAM_COL * SM511_2_RAM_LINE, file);
    
    // Clock divider and flags
    fwrite(&f_clock_divider, sizeof(uint16_t), 1, file);
    fwrite(&gamma_flag_second, sizeof(bool), 1, file);
    
    // Melody state
    fwrite(&rom_melody_address, sizeof(uint8_t), 1, file);
    fwrite(&me_melody_activate, sizeof(bool), 1, file);
    
    return true;
}

// Load SM511_2 CPU state from file
bool SM511_2::load_state(FILE* file) {
    if(!file) return false;
    
    // Load CPU state
    if(fread(&program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&s_buffer_program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&r_buffer_program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&ram_address, sizeof(RAMAddress), 1, file) != 1) return false;
    if(fread(&accumulator, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&carry, sizeof(bool), 1, file) != 1) return false;
    
    // Load RAM
    if(fread(ram, sizeof(uint8_t), SM511_2_RAM_COL * SM511_2_RAM_LINE, file) != SM511_2_RAM_COL * SM511_2_RAM_LINE) return false;
    
    // Load clock divider
    if(fread(&f_clock_divider, sizeof(uint16_t), 1, file) != 1) return false;
    if(fread(&gamma_flag_second, sizeof(bool), 1, file) != 1) return false;
    
    // Load melody state
    if(fread(&rom_melody_address, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&me_melody_activate, sizeof(bool), 1, file) != 1) return false;
    
    // Update segments so display reflects loaded RAM
    update_segment();
    segments_state_are_update = true;
    
    return true;
}
