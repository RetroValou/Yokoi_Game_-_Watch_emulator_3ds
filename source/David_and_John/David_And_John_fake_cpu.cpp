#include "std/timer.h"
#include <cstring>
#include "./David_And_John_fake_cpu.h"
#include "./David_and_John_program.h"

#include "./Climber/D_And_J_Climber_program.h"
#include "./Avignon/D_And_J_Avignon_program.h"
#include "./Esquive/D_And_J_Esquive_program.h"



void David_And_John_fake_cpu::load_rom(const uint8_t* file_hex, size_t size_hex){    
     
    switch (file_hex[size_hex-1])
    {
        case 1:
            curr_program = new D_And_J_Climber_program(this);
            break;
        case 2:
            curr_program = new D_And_J_Avignon_program(this);
            break;
        case 3:
            break;
        case 4:
            break;
        case 5:
            curr_program = new D_And_J_Esquive_program(this);
            break;
    }
};

void David_And_John_fake_cpu::init(){
    current_score = 0;
    first_loop = true;
    play_note = false;

    program_counter = {0, 0x0F, 0}; // Doc Sharp
    s_buffer_program_counter = {0, 0, 0};

    ram_address = {0, 0}; // not indicate in doc Sharp

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }

    carry = 0;
    accumulator = 0;

    f_clock_divider = 0x0002;    
    gamma_flag_second = true;

    bp_lcd_blackplate = true;
    is_sleep = false;

    segments_state_are_update = false;
    cycle_curr_opcode = 0;

    flag_time_update_screen = 0x00;
    cpu_frequency_divider = 0x01;

    for(int i = 0; i < 8; i++){ k_input[i] = 0x00; }

    stop_cpu = false;
    sound_divide_frequency = 1;

    is_during_wait_mark = false;

    show_all_segment = false;
}


void David_And_John_fake_cpu::execute_curr_opcode(){
    segments_state_are_update = true;
    if(first_loop){ 
        if(make_mark()){ curr_program->init_program(); first_loop = false; }
    }
    else { curr_program->execute_program_loop(); }
}

bool David_And_John_fake_cpu::get_segments_state(uint8_t col, uint8_t line, uint8_t word){
    if(!screen_is_on()){ return false; }
    if(show_all_segment){ return true; }
    return curr_program->get_segments_state(line, word);
};





// Save/Load state
bool David_And_John_fake_cpu::save_state(FILE* file){
    if(!file) return false;
        
    fwrite(&high_score, sizeof(uint64_t), 1, file);
    return true;
};

bool David_And_John_fake_cpu::load_state(FILE* file){
    if(!file) return false;
    
    if(fread(&high_score, sizeof(uint64_t), 1, file) != 1) return false;
    return true;
};


bool David_And_John_fake_cpu::make_mark(){
    if(!is_during_wait_mark){
        is_during_wait_mark = true;
        wait_make_mark = time_us_64_p();
    }

    if(wait_make_mark + 300000 > time_us_64_p()){
        bp_lcd_blackplate = false;
    }
    else if(wait_make_mark + 400000 > time_us_64_p()){
        bp_lcd_blackplate = true;
        show_all_segment = true;
    }
    else if(wait_make_mark + 550000 > time_us_64_p()){
        bp_lcd_blackplate = false;
    }
    else {
        show_all_segment = false;
        bp_lcd_blackplate = true;
        is_during_wait_mark = false;
    }
    return !is_during_wait_mark;
}


void David_And_John_fake_cpu::end_of_cpu() { 
    curr_program->stop_program(); 
    delete curr_program;
}



bool David_And_John_fake_cpu::get_input(uint8_t group, uint8_t line){
    // special input -> says by line >= 8 (not exist in true K input)
    if(line == 8){ return alpha_input; }
    else if(line == 9){ return beta_input; }
    return ((k_input[group] >> line) & 0x01) == 0x01;
}


/////// SOUND ///////////////////////////////////////////////////////////////



bool David_And_John_fake_cpu::get_active_sound(){
    return play_note && !sound_wait_silence && sound_curr_bit;
}

void David_And_John_fake_cpu::update_sound(){
    if(play_note){
        if(!sound_wait_silence){
            update_note();
            if(sound_curr_time + sound_time_length < time_us_64_p() && sound_curr_index == 0){
                if(sound_curr_bip == sound_nb_bip)
                    { play_note = false; return; } // finish
                sound_wait_silence = true; 
                sound_curr_time = time_us_64_p();
            }
        }
        else {
            if(sound_curr_time + WAIT_BETWEEN_BIP < time_us_64_p()){
                sound_wait_silence = false; 
                sound_curr_index = 0; sound_curr_bit = 1;
                sound_curr_bip += 1;
                sound_curr_time = time_us_64_p();
            }
        }
    }
}

void David_And_John_fake_cpu::update_note(){
    sound_curr_index += 1;
    if(sound_curr_index >= sound_note){
        sound_curr_bit = !sound_curr_bit;
        sound_curr_index = 0;
    }
}


void David_And_John_fake_cpu::play_sound(uint8_t note, uint64_t time_note, uint8_t nb_bip){
    sound_note = note;
    sound_time_length = time_note;
    sound_nb_bip = nb_bip;

    // init
    sound_wait_silence = false;
    play_note = true;
    sound_curr_time = time_us_64_p();

    sound_curr_index = 0;
    sound_curr_bip = 1;
    sound_curr_bit = true;
}




/////// DEBUG ///////////////////////////////////////////////////////////////

void David_And_John_fake_cpu::set_beat_value_for_debug(){
    ram_address.col = (curr_program->beat)->new_index;
    ram_address.line = (curr_program->beat)->index_now;
}
