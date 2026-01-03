#include "SM5A.h"
#include "std/timer.h"
#include <stdio.h>
#include <string.h>

// Save SM5A CPU state to file
bool SM5A::save_state(FILE* file) {
    if(!file) return false;
    
    // Save critical CPU state needed to resume execution
    
    // Program counter - WHERE the CPU is in the code
    fwrite(&program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&s_buffer_program_counter, sizeof(ProgramCounter), 1, file);
    fwrite(&ram_address, sizeof(RAMAddress), 1, file);
    
    // Accumulator and carry - current calculation state
    fwrite(&accumulator, sizeof(uint8_t), 1, file);
    fwrite(&carry, sizeof(bool), 1, file);
    
    // RAM - this holds the game state (score, lives, positions, etc.)
    fwrite(ram, sizeof(uint8_t), SM5A_RAM_COL * SM5A_RAM_LINE, file);
    
    // Screen control registers - needed to restore display
    fwrite(w_screen_control, sizeof(uint8_t), 9, file);
    fwrite(w_prime_screen_control, sizeof(uint8_t), 9, file);
    
    // Clock divider - needed for game timing
    fwrite(&f_clock_divider, sizeof(uint16_t), 1, file);
    fwrite(&gamma_flag_second, sizeof(bool), 1, file);
    
    // SM5A-specific flags that affect execution
    fwrite(&cn_flag, sizeof(bool), 1, file);
    fwrite(&r_subroutine_flag, sizeof(bool), 1, file);
    fwrite(&e_temporar_flag, sizeof(bool), 1, file);
    fwrite(&m_flag_segment_decoder, sizeof(bool), 1, file);
    
    return true;
}

// Load SM5A CPU state from file
bool SM5A::load_state(FILE* file) {
    if(!file) return false;
    
    // Load program counter - WHERE to resume execution
    if(fread(&program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&s_buffer_program_counter, sizeof(ProgramCounter), 1, file) != 1) return false;
    if(fread(&ram_address, sizeof(RAMAddress), 1, file) != 1) return false;
    
    // Load accumulator and carry
    if(fread(&accumulator, sizeof(uint8_t), 1, file) != 1) return false;
    if(fread(&carry, sizeof(bool), 1, file) != 1) return false;
    
    // Load RAM
    if(fread(ram, sizeof(uint8_t), SM5A_RAM_COL * SM5A_RAM_LINE, file) != SM5A_RAM_COL * SM5A_RAM_LINE) return false;
    
    // Load screen control registers
    if(fread(w_screen_control, sizeof(uint8_t), 9, file) != 9) return false;
    if(fread(w_prime_screen_control, sizeof(uint8_t), 9, file) != 9) return false;
    
    // Load clock divider
    if(fread(&f_clock_divider, sizeof(uint16_t), 1, file) != 1) return false;
    if(fread(&gamma_flag_second, sizeof(bool), 1, file) != 1) return false;
    
    // Load SM5A-specific flags
    if(fread(&cn_flag, sizeof(bool), 1, file) != 1) return false;
    if(fread(&r_subroutine_flag, sizeof(bool), 1, file) != 1) return false;
    if(fread(&e_temporar_flag, sizeof(bool), 1, file) != 1) return false;
    if(fread(&m_flag_segment_decoder, sizeof(bool), 1, file) != 1) return false;
    
    // Update segments so display reflects loaded state
    update_segment();
    segments_state_are_update = true;
    
    return true;
}
