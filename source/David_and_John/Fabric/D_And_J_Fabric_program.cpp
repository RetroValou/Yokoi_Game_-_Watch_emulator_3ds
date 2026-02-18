#include "D_And_J_Fabric_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>


bool D_And_J_Fabric_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // Dead
            if(curr_phase == Program_Phase::Dead){ return (word == david_before[0]); }
            return false;

        case 1: // player
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            if(curr_phase == Program_Phase::Score_life_adding)
                { return (word == pos_player[0] + pos_player[1]*PLAYER_LOC_f[0] + player_get_item*PLAYER_LOC_f[0]*PLAYER_LOC_f[1]); }
            return (word == pos_player[0] + pos_player[1]*PLAYER_LOC_f[0] + player_get_item*PLAYER_LOC_f[0]*PLAYER_LOC_f[1]) 
                                && show_player;
        
        case 2: // projectile
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x + (*list_projectiles)[i].pos_y*PLAYER_LOC_f[0]){
                    if((*list_projectiles)[i].used 
                    && (*list_projectiles)[i].curr_state == Projectile_state::down){ return true; }
                }
            }
            return false;

        case 3: // projectile before
            return (word == david_before[0] + david_before[1]*PLAYER_LOC_f[0]);

        case 4: // decors
            return true;

        case 5: // life
            return segment_life(word);

        case 6: case 7: case 8: case 9: // scores
            return segment_score(word, line-6);

    }
    return false;
}
