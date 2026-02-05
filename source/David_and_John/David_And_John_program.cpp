#include "David_And_John_program.h"
#include "std/timer.h"
#include "David_And_John_fake_cpu.h"


bool David_And_John_program::player_autorize_to_move(bool input_result){
    if(input_result){
        uint64_t curr_time = time_us_64_p();
        if(last_time_player_move + WAIT_BEFORE_PLAYER_MOVE < curr_time
                || curr_time < last_time_player_move){ // for overflow
            last_time_player_move = curr_time;
            return true;
        }
    }
    return false;
}


void David_And_John_program::update_step(){
    uint64_t curr_time = time_us_64_p();
    if(last_time_step + WAIT_BEFORE_STEP < curr_time
            || curr_time < last_time_step){ // for overflow
        last_time_step = curr_time;
        beat->update();
        fake_cpu->set_beat_value_for_debug();
        additional_update_step();
    }
}



void David_And_John_program::blink_player(){
    if(last_wait_x_blink + WAIT_BLINK < time_us_64_p()  
        || last_wait_x_blink > time_us_64_p()){
        last_wait_x_blink = time_us_64_p();
        show_player = !show_player;
    }
}

void David_And_John_program::stop_blink_player(){
    show_player = true;
}


void David_And_John_program::blink_last_life(){
    if(last_wait_x_blink + WAIT_BLINK < time_us_64_p()  
        || last_wait_x_blink > time_us_64_p()){
        last_wait_x_blink = time_us_64_p();
        show_last_life = !show_last_life;
    }
}

void David_And_John_program::stop_blink_last_life(){
    show_last_life = true;
}


