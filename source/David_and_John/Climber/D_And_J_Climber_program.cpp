#include "D_And_J_Climber_program.h"
#include <cstdint>
#include <string>


bool D_And_J_Climber_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
            return word == pos_player[0]+pos_player[1]*PLAYER_LOC_X; break;
        case 1: // wall
            return wall[word]; break;
    }
    return false;
}

void D_And_J_Climber_program::execute_program_loop(){
    
}

void D_And_J_Climber_program::init_program(){
}

