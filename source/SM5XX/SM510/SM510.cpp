#include "SM5XX/SM510/SM510.h"
#include "SM5XX/sm510/sm510.h"
#include "std/timer.h"
#include <cstring>
#include <stdio.h>



void SM510::init()
{
    program_counter = {3, 7, 0}; // Doc Sharp
    s_buffer_program_counter = {0, 0, 0};
    r_buffer_program_counter = {0, 0, 0};

    for(int col = 0; col < SM510_RAM_COL; ++col){
        for(int line = 0; line < SM510_RAM_LINE; ++line){ ram[col][line] = 0; } }
    ram_address = {0, 0}; // not indicate in doc Sharp

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }
    w_shift_register = 0;

    alpha_input = true;
    beta_input = true;

    carry = 0;
    accumulator = 0;

    f_clock_divider = 0x0001;    
    gamma_flag_second = false;
    
    r_buzzer_control = 0x00;
    r_buzzer_output = 0x00;

    bp_lcd_blackplate = true;
    bc_lcd_stop = false;

    l_bs = 0x00;
    y_bs = 0x00; 

    is_sleep = false;

    segments_state_are_update = false;
    time_last_group_cycle = time_us_64_p();
    nb_group_cycle = 0;
    alternativ_col_ram = 0x00; // used for sbm -> change temporaly value of ram col adresse
    cycle_curr_opcode = 0;

    for(int col = 0; col < SM510_RAM_VIDEO_COL+1; col++){
        for(int line = 0; line < SM510_RAM_LINE; line++){ segment_on[col][line] = 0x00; }
    }
    flag_time_update_screen = 0x00;

    cpu_frequency_divider = 0x01;

    stop_cpu = false;

    sound_divide_frequency = 1; //8
}


bool SM510::no_pc_increase(uint8_t opcode){
    // In """theorie""" of sharp doc : PC not increase if jump opcode
    // I pratic is more complicated
    return //((opcode & 0xC0) == 0xC0) // TM is jump but insert incremented pc in buffer before erase pc value
           ((opcode & 0xC0) == 0x80) // T. Technicly no impact if increase because erase by opcode
           | (opcode == 0x03) // ATPL. Only very need to not execute PC increase
           // | ((opcode & 0xF0) == 0x70) // TML or TL are jump but parameter need to keep after by pc increase
           | ((opcode & 0xFE) == 0x6E) // RTN0 or RTN1. Technicly no impact if increase because erase by opcode
    ;
}

bool SM510::is_on_double_octet(uint8_t opcode){
    // opcode need next opcode for work -> like parameter
    return ( opcode == 0x5F ) // LBL
            | ( (opcode & 0xF0) == 0x70 ) // TML or TL
    ;
}



/////////////////////////// RAM / ROM MANIPULATION //////////////////////////////////////////////////////

void SM510::load_rom(const uint8_t* file_hex, size_t){
    for(int col = 0; col < SM510_ROM_COL; col++){ 
        for(int line = 0; line < SM510_ROM_LINE; line++){
            for(int word = 0; word < ROM_WORD; word++){
                rom[col][line][word] = file_hex[SM510_ROM_MAME_LINE * SM510_ROM_MAME_WORD * col 
                                                    + SM510_ROM_MAME_WORD * line 
                                                    + word];

            }
        }
    }
}


// Read / write data -> from rom or ram
uint8_t SM510::read_rom_value(){
    return rom[program_counter.col][program_counter.line][program_counter.word];
}


uint8_t SM510::read_ram_value(){
    uint8_t col = ram_address.col;
    if(alternativ_col_ram != 0x00){ col = alternativ_col_ram;} // x[1] bit are upper -> active

    col = min(col, SM510_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line = min(ram_address.line, SM510_RAM_LINE-1);
    return ram[col][line];
}

void SM510::write_ram_value(uint8_t value){
    uint8_t col = ram_address.col;
    if(alternativ_col_ram != 0x00){ col = alternativ_col_ram;} // x[1] bit are upper -> active
    
    col = min(col, SM510_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line = min(ram_address.line, SM510_RAM_LINE-1);
    ram[col][line] = value & 0x0F; // 4 bit RAM
}

void SM510::set_ram_value(uint8_t col, uint8_t line, uint8_t value) {
    if (col >= SM510_RAM_COL || line >= SM510_RAM_LINE) 
        return; 
    ram[col][line] = value;
}

/////////////////////////// Wake up //////////////////////////////////////////////////////

void SM510::wake_up(){
    is_sleep = false;
    program_counter = {1, 0, 0}; // Doc Sharp
    bp_lcd_blackplate = true; // start screen
    bc_lcd_stop = false;
}


/////////////////////////// Segment / screen //////////////////////////////////////////////////////

bool SM510::condition_to_update_segment(){
    uint16_t value = ((f_clock_divider >> 9) & 0x01);
    if( value != flag_time_update_screen)
    { // simplify version for perf
        flag_time_update_screen = value;
        return true;
    }
    return false;
}


bool SM510::screen_is_on() { return (!bc_lcd_stop) && bp_lcd_blackplate; }

void SM510::update_segment(){
    for(int col = 0; col < SM510_RAM_VIDEO_COL+1; col++){
        // Col 0 -> i_Ram 6 -> Pin A
        // Col 1 -> i_Ram 7 -> Pin B
        // Col 2 -> (l_bs + x_bs) with Y_bs -> Pin bs (used for blink segment or increase nb segments)
        // Col 3 -> i_Ram 5 -> Pin C // Only on SM512
        // Col N -> i_ram - N -> theorie not exist
        int ram_col;
        switch (col)
        {
            case 0: case 1: ram_col = col + (SM510_RAM_COL-2); break;
            case 2: ram_col = -1; break;
            default: ram_col = SM510_RAM_COL-col; break;
        }
        for(int line = 0; line < SM510_RAM_LINE; line++){
            if(ram_col == -1){ // bs input
                segment_on[col][line] = segment_on_value_sp_bs(line); }
            else { segment_on[col][line] = ram[ram_col][line];} 
        }
    }
    segments_state_are_update  = true;
}

uint8_t SM510::segment_on_value_sp_bs(int curr_line){
    uint8_t blink = 0xFF; // calculate blink -> default not blink
    if(((f_clock_divider >> 14)&0x01) == 0x01) // (0.5sec off, 0.5sec on)
        { blink = ~y_bs; } // theorie base on doc and rom observation 
                            // who y_bs used for bs but not connected directly. 
                            // When y_bs = 0, it's probably for "desactivate blink effect"
    if(curr_line == 0){ return l_bs & blink; }
    return 0x00;
}


bool SM510::get_segments_state(uint8_t col, uint8_t line, uint8_t word) {
    if(col >= (SM510_RAM_VIDEO_COL+1) || line >= SM510_RAM_LINE || word >= 4){ return false; }
    return (segment_on[col][line] >> word)&0x01; }


/////////////////////////// Sound //////////////////////////////////////////////////////


void SM510::update_sound(){
    /*if((f_clock_divider & 0x07) == 0x07){ //frequency of 4 054Khz
        r_buzzer_output = r_buzzer_control;
    }*/
}

bool SM510::get_active_sound(){
    //return (r_buzzer_control & 0x01) | ((r_buzzer_control >> 1) & 0x01);
    //return (r_buzzer_output & 0x01) | ((r_buzzer_output >> 1) & 0x01);
    //if( ((f_clock_divider>>3)& 0x01) == 0x01){ return (r_buzzer_control & 0x01); }
    if( ((f_clock_divider>>2)& 0x01) == 0x01){ return (r_buzzer_control & 0x01); }
    return 0x00/*((r_buzzer_control >> 1) & 0x01)*/;
}


/////////////////////////// Debug //////////////////////////////////////////////////////
uint8_t SM510::debug_get_elem_ram(int col, int line) { 
    uint8_t col_ = min(col, SM510_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line_ = min(line, SM510_RAM_LINE-1);
    return ram[col_][line_]; 
}

