#include "SM5XX/SM511_SM512/SM511_2.h"
#include "SM5XX/SM511_SM512/sm511_2.h"
#include "std/timer.h"
#include <cstring>



void SM511_2::init()
{
    program_counter = {3, 7, 0}; // Doc Sharp
    s_buffer_program_counter = {0, 0, 0};
    r_buffer_program_counter = {0, 0, 0};

    for(int col = 0; col < SM511_2_RAM_COL; ++col){
        for(int line = 0; line < SM511_2_RAM_LINE; ++line){ ram[col][line] = 0; } }
    ram_address = {0, 0}; // not indicate in doc Sharp

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }
    w_shift_register = 0;

    beta_input = true;
    alpha_input = true;

    carry = 0;
    accumulator = 0;

    f_clock_divider = 0x0001;    
    gamma_flag_second = false;
    
    me_melody_activate = true;
    mes_melody_finish = false;
    nb_cycle_need_for_change_note = (uint16_t)(SM511_2_TIME_CHANGE_NOTE * FREQUENCY_CPU / 1'000'000);
    uint8_t init_add = 0x00;
    load_new_note(&init_add);

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

    for(int line = 0; line < SM511_2_RAM_LINE;line++){
        for(int col = 0; col < SM511_2_RAM_VIDEO_COL+1; col++){ segment_on[col][line] = 0x00; }
    }
    flag_time_update_screen = 0x00;

    cpu_frequency_divider = 2; //4*8.192kHz = 2*16.384kHz -> 32.768kHz (true frequency of CPU)
                // MAME and Sharp doc says frequency are 16.384kHz 
                // because instruction take 2 cycles to execute (4 for some)

    stop_cpu = false;
    //sound_frequency = FREQUENCY_CPU;
}


bool SM511_2::no_pc_increase(uint8_t opcode){
    // In """theorie""" of sharp doc : PC not increase if jump opcode
    // I pratic is more complicated
    return 
           ((opcode & 0xC0) == 0x80) // T. Technicly no impact if increase because erase by opcode
           | (opcode == 0x03) // ATPL. Only very need to not execute PC increase
           | ((opcode & 0xFE) == 0x6E) // RTN0 or RTN1. Technicly no impact if increase because erase by opcode
            // TML or TL are jump but parameter need to keep after by pc increase
            // TM is jump but insert incremented pc in buffer before erase pc value
           ;
}

bool SM511_2::is_on_double_octet(uint8_t opcode){
    // opcode need next opcode for work -> like parameter
    return ( opcode == 0x5F ) // LBL
            | ( (opcode & 0xF0) == 0x70 ) // TL
            | ((opcode & 0xfc) == 0x68) // tml
            | (opcode == 0x60) // rme, sme, tmel, atfc, bdc, atbp, clkhi, clklo and illegal opcode (in bad case)
            | (opcode == 0x61) // pre
    ;
}


/////////////////////////// RAM / ROM MANIPULATION //////////////////////////////////////////////////////

void SM511_2::load_rom(const uint8_t* file_hex, size_t){
    for(int col = 0; col < SM511_2_ROM_COL; col++){ 
        for(int line = 0; line < SM511_2_ROM_LINE; line++){
            for(int word = 0; word < ROM_WORD; word++){
                rom[col][line][word] = file_hex[SM511_2_ROM_MAME_LINE * SM511_2_ROM_MAME_WORD * col 
                                                    + SM511_2_ROM_MAME_WORD * line 
                                                    + word];
            }
        }
    }
}

void SM511_2::load_rom_melody(const uint8_t* file_hex, size_t){
    for(int word = 0; word < 256; word++){ rom_melody[word] = file_hex[word]; }
}

// Read / write data -> from rom or ram
uint8_t SM511_2::read_rom_value(){
    return rom[program_counter.col][program_counter.line][program_counter.word];
}
uint8_t SM511_2::read_rom_melody_value(){
    return rom_melody[rom_melody_address];
}

uint8_t SM511_2::read_ram_value(){
    uint8_t col = ram_address.col;
    if(alternativ_col_ram != 0x00){ col = alternativ_col_ram;} // x[1] bit are upper -> active

    col = min(col, SM511_2_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line = min(ram_address.line, SM511_2_RAM_LINE-1);
    return ram[col][line];
}

void SM511_2::write_ram_value(uint8_t value){
    uint8_t col = ram_address.col;
    if(alternativ_col_ram != 0x00){ col = alternativ_col_ram;} // x[1] bit are upper -> active

    col = min(col, SM511_2_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line = min(ram_address.line, SM511_2_RAM_LINE-1);
    ram[col][line] = value & 0x0F; // 4 bit RAM
}

void SM511_2::set_ram_value(uint8_t col, uint8_t line, uint8_t value) {
    if (col >= SM511_2_RAM_COL || line >= SM511_2_RAM_LINE) 
        return;
    ram[col][line] = value;
}

/////////////////////////// Wake up //////////////////////////////////////////////////////

void SM511_2::wake_up(){
    is_sleep = false;
    program_counter = {1, 0, 0}; // Doc Sharp
    bp_lcd_blackplate = true; // start screen
    bc_lcd_stop = false;
}


/////////////////////////// Segment / screen //////////////////////////////////////////////////////

bool SM511_2::condition_to_update_segment(){
    uint16_t value = ((f_clock_divider >> 9) & 0x01);
    if( value != flag_time_update_screen)
    { // simplify version for perf
        flag_time_update_screen = value;
        return true;
    }
    return false;
}

bool SM511_2::screen_is_on() { return (!bc_lcd_stop) && bp_lcd_blackplate; }

void SM511_2::update_segment(){
    for(int col = 0; col < SM511_2_RAM_VIDEO_COL+1; col++){
        // Col 0 -> i_Ram 6 -> Pin A
        // Col 1 -> i_Ram 7 -> Pin B
        // Col 2 -> (l_bs + x_bs) with Y_bs -> Pin bs (used for blink segment or increase nb segments)
        // Col 3 -> i_Ram 5 -> Pin C // Only on SM512
        // Col N -> i_ram - N -> theorie not exist
        int ram_col;
        switch (col)
        {
            case 0: case 1: ram_col = col + (SM511_2_RAM_COL-2); break;
            case 2: ram_col = -1; break;
            default: ram_col = SM511_2_RAM_COL-col; break;
        }
        for(int line = 0; line < SM511_2_RAM_LINE; line++){
            if(ram_col == -1){ // bs input
                segment_on[col][line] = segment_on_value_sp_bs(line); }
            else { segment_on[col][line] = ram[ram_col][line];} 
        }
    }
    segments_state_are_update  = true;
}

uint8_t SM511_2::segment_on_value_sp_bs(int curr_line){
    uint8_t blink = 0xFF; // calculate blink -> default not blink
    if(((f_clock_divider >> 14)&0x01) == 0x01) // (0.5sec off, 0.5sec on)
        { blink = ~y_bs; } // theorie base on doc and rom observation 
                            // who y_bs used for bs but not connected directly. 
                            // When y_bs = 0, it's probably for "desactivate blink effect"
    if(curr_line == 0){ return l_bs & blink; }
    else if(curr_line == 1){ return x_bs & blink; }
    return 0x00;
}


bool SM511_2::get_segments_state(uint8_t col, uint8_t line, uint8_t word) {
    if(col >= (SM511_2_RAM_VIDEO_COL+1) || line >= SM511_2_RAM_LINE || word >= 4){ return false; }
    return (segment_on[col][line] >> word)&0x01; }


/////////////////////////// Melody system //////////////////////////////////////////////////////

void SM511_2::load_new_note(uint8_t* update_melody_adress){
    melody_cycle_count = 0;
    curr_phase = 0;
    cycle_in_curr_phase = 0;

    if(update_melody_adress == nullptr){ rom_melody_address += 1; }
    else { rom_melody_address = *update_melody_adress; }

    uint8_t data = read_rom_melody_value();
    curr_note_melody.note = (data & 0x0F);
    curr_note_melody.double_time = ((data & 0x20) == 0x20);
    curr_note_melody.octave = ((data & 0x10) == 0x10);
}


void SM511_2::update_sound(){ // NOT FINISH
    if(!me_melody_activate){ return; }
    
    // execute of each cyle
    static const uint8_t note_frequency_control[4*12]{ // Doc Sharp SM511 + MAME
            7,  8,  8,  8 // do
            , 8,  8,  8,  8 // si
            , 8,  9,  9,  9 // la #
            , 9,  9,  9, 10 // la
            , 9, 10, 10, 10 // so #
            ,10, 11, 10, 11 // so
            ,11, 11, 11, 11 // fa #
            ,11, 12, 12, 12 // fa
            ,12, 13, 12, 13 // mi
            ,13, 13, 13, 14 // re #
            ,14, 14, 14, 14 // re
            ,14, 15, 15, 15 // do #
        };
/*
    static const uint32_t output_sound_frequency[4*12]{
            2114, 1986, 1872, 1771, 1680, 1560, 1490, 1394, 1311, 1236, 1170, 1111 };
           //do / si /  la #  /la  / so #/ so / fa # / fa /  mi  / re # / re / do #
*/


    if( (curr_note_melody.note) == 0x00) {  // nothing
        curr_phase = 0x00;         
        //sound_frequency = 0; 
    }
    else if( (curr_note_melody.note) == 0x01) { // stop melody
        curr_phase = 0x00; 
        //sound_frequency = 0; 
        mes_melody_finish = true; 
    }
    else if((curr_note_melody.note) <= 13) // only 12 notes
    { 
        //sound_frequency = output_sound_frequency[curr_note_melody.note-2];
        int ind_note_freq = (curr_note_melody.note-2)*4 + curr_phase;
        if( cycle_in_curr_phase >= (note_frequency_control[ind_note_freq] * (curr_note_melody.octave? 2: 1))){
            cycle_in_curr_phase = 0;
            curr_phase = (curr_phase+1) & 0x03;
        }
        cycle_in_curr_phase += 1;
    }

    // see if moving or not
    melody_cycle_count += 1;
    if( ( (!curr_note_melody.double_time) // not double time
            && (melody_cycle_count >= nb_cycle_need_for_change_note)) // not double time
        || (melody_cycle_count >= 2*nb_cycle_need_for_change_note)) // double time
    {
        load_new_note();
    }
}


bool SM511_2::get_active_sound(){
    return me_melody_activate && ((curr_phase&0x01) == 0x01); // phase impair -> Activate buzzer
}


/////////////////////////// Debug //////////////////////////////////////////////////////
uint8_t SM511_2::debug_get_elem_ram(int col, int line) { 
    uint8_t col_ = min(col, SM511_2_RAM_COL-1); // copy of max value if col and line too big
    uint8_t line_ = min(line, SM511_2_RAM_LINE-1);
    return ram[col_][line_]; 
}

