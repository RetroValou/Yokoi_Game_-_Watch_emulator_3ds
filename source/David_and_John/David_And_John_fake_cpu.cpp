#include "std/timer.h"
#include <cstring>
#include "David_and_John/David_And_John_fake_cpu.h"
#include "David_and_John/David_and_John_program.h"
#include "David_and_John/Climber/D_And_J_Climber_program.h"


void David_And_John_fake_cpu::load_rom(const uint8_t* file_hex, size_t size_hex){     
    switch (file_hex[size_hex-1])
    {
        case 1:
            curr_program = new D_And_J_Climber_program(this);
            break;
    }
};

void David_And_John_fake_cpu::init(){
    current_score = 0;
    first_loop = true;

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

bool David_And_John_fake_cpu::get_active_sound(){
    return false;
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
