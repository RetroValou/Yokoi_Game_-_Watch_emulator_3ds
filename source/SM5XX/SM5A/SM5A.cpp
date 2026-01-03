#include "SM5XX/SM5A/SM5A.h"
#include "SM5XX/SM5A/sm5A.h"
#include "std/timer.h"
#include <cstring>


void SM5A::init()
{
    program_counter = {0, 0x0F, 0}; // Doc Sharp
    s_buffer_program_counter = {0, 0, 0};
    cb_debordement_rom_program_counter = 0x00;

    for(int col = 0; col < SM5A_RAM_COL; ++col){
        for(int line = 0; line < SM5A_RAM_LINE; ++line){ ram[col][line] = 0; } }
    ram_address = {0, 0}; // not indicate in doc Sharp

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }
    r_output_control = 0xFF; // inversed !

    beta_input = true;
    alpha_input = true;

    carry = 0;
    accumulator = 0;

    f_clock_divider = 0x0002;    
    gamma_flag_second = true;

    r_subroutine_flag = false;
    e_temporar_flag = false;
    m_flag_segment_decoder = false;

    bp_lcd_blackplate = false;
    cn_flag = false;

    is_sleep = false;

    for(int i = 0; i < 9; i++){
        w_screen_control[i] = 0x00;
        w_prime_screen_control[i] = 0x00;
    }

    segments_state_are_update = false;
    time_last_group_cycle = time_us_64_p();
    nb_group_cycle = 0;
    cycle_curr_opcode = 0;

    for(int col = 0; col < SM5A_SEGMENT_COL; col++){
        for(int line = 0; line < SM5A_SEGMENT_LINE; line++){ segment_on[col][line] = 0x00; }
    }
    flag_time_update_screen = 0x00;
    last_w_update = 0x00;

    cpu_frequency_divider = 0x01;

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }

    stop_cpu = false;

    sound_divide_frequency = 1;
    //sound_frequency = FREQUENCY_CPU / 8;
}


bool SM5A::no_pc_increase(uint8_t opcode){
    // In """theorie""" of sharp doc : PC not increase if jump opcode
    // I pratic is more complicated
    return //((opcode & 0xC0) == 0xC0) // TM is jump but insert incremented pc in buffer before erase pc value
           ((opcode & 0xC0) == 0x80) // TR. Technicly no impact if increase because erase by opcode
           // | ((opcode & 0xF0) == 0x70) // TML or TL are jump but parameter need to keep after by pc increase
           | ((opcode & 0xFE) == 0x6E) // RTN or RTNS. Technicly no impact if increase because erase by opcode
    ;
}						


bool SM5A::is_on_double_octet(uint8_t opcode){
    // opcode need next opcode for work -> like parameter
    return ( opcode == 0x5F ) // LBL
            | ( opcode == 0x5E ) // DTA+cend
    ;
}


/////////////////////////// RAM / ROM MANIPULATION //////////////////////////////////////////////////////

void SM5A::load_rom(const uint8_t* file_hex, size_t size_hex){
    for(size_t col = 0; col < SM5A_ROM_COL; col++){ 
        for(size_t line = 0; line < SM5A_ROM_LINE; line++){
            for(size_t word = 0; word < ROM_WORD; word++){
                size_t index = SM5A_ROM_MAME_LINE * SM5A_ROM_MAME_WORD * col + SM5A_ROM_MAME_WORD * line + word;
                if(index >= size_hex){ rom[col][line][word] = 0x00; } // if rom are full -> empty
                else { rom[col][line][word] = file_hex[index]; }
            }
        }
    }
}


// Read / write data -> from rom or ram
uint8_t SM5A::read_rom_value(){
    return rom[program_counter.col][program_counter.line][program_counter.word];
}

uint8_t SM5A::read_ram_value(){
    uint8_t col = min(ram_address.col, SM5A_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line = min(ram_address.line, SM5A_RAM_LINE-1);
    return ram[col][line];
}

void SM5A::write_ram_value(uint8_t value){
    uint8_t col = min(ram_address.col, SM5A_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line = min(ram_address.line, SM5A_RAM_LINE-1);
    ram[col][line] = (value & 0x0F);
}

void SM5A::set_ram_value(uint8_t col, uint8_t line, uint8_t value) {
    if (col >= SM5A_RAM_COL || line >= SM5A_RAM_LINE) 
        return;
    ram[col][line] = value;
}



/////////////////////////// Wake up //////////////////////////////////////////////////////

void SM5A::wake_up(){
    is_sleep = false;
    program_counter = {0, 0, 0}; // Doc Sharp
    bp_lcd_blackplate = true; // start screen
}


/////////////////////////// Segment / screen //////////////////////////////////////////////////////

bool SM5A::condition_to_update_segment(){
    if(last_w_update != 0){ last_w_update -= 1; } // not during maj w -> prevention of glitch
    
    uint16_t value = ((f_clock_divider >> 9) & 0x01);
    if( (value != flag_time_update_screen) && last_w_update == 0)
    { // simplify version for perf
        flag_time_update_screen = value;
        return true;
    }
    return false;
}

bool SM5A::screen_is_on() { return bp_lcd_blackplate; }


void SM5A::update_segment(){
    // COL = index w and w_prime
    // Line = bit of w and w_prime
    // word = w or w_prime
    for(int col = 0; col < SM5A_SEGMENT_COL; col++){
        for(int line = 0; line < SM5A_SEGMENT_LINE; line++){
            segment_on[col][line] = ((w_screen_control[col]>>line)&0x01) 
                                    | (((w_prime_screen_control[col]>>line)&0x01) << 1);
        }
    }
    segments_state_are_update  = true;
}


bool SM5A::get_segments_state(uint8_t col, uint8_t line, uint8_t word) {
    if(col >= SM5A_SEGMENT_COL || line >= SM5A_SEGMENT_LINE || word >= SM5A_SEGMENT_WORD){ return false; }
    return (segment_on[col][line] >> word)&0x01; }


/////////////////////////// Sound //////////////////////////////////////////////////////

bool SM5A::get_active_sound(){
    return (r_output_control & 0x01) == 0x00;
}


/////////////////////////// Debug //////////////////////////////////////////////////////
uint8_t SM5A::debug_get_elem_ram(int col, int line) { 
    uint8_t col_ = min(col, SM5A_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line_ = min(line, SM5A_RAM_LINE-1);
    return ram[col_][line_];
}
