#include "David_And_John_program.h"
#include "std/timer.h"


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
        additional_update_step();
    }
}
