#include "D_And_J_Climber_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"


bool D_And_J_Climber_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            else if(curr_phase == Program_Phase::Sucess){ return (word == 12); }
            return word == pos_player[0]+pos_player[1]*PLAYER_LOC[0];
        case 1: // wall
            return wall[word];

        case 2: // projectile
            for(int i = 0; i < MAX_PROJECTILES; i++){
                if(word == list_projectiles[i]->pos_x + list_projectiles[i]->pos_y*PLAYER_LOC[0]){
                    if(list_projectiles[i]->used 
                    && list_projectiles[i]->curr_state == Projectile_state::down){ return true; }
                }
            }
            return false;

        case 3: // projectile wait before
            // if one of projectile is show before fall, not show wait before
            for(int i = 0; i < MAX_PROJECTILES; i++){
                if(list_projectiles[i]->used &&
                    list_projectiles[i]->curr_state == Projectile_state::before)
                { return false; }
            }
            return true;

        case 4: // projectile before
            for(int i = 0; i < MAX_PROJECTILES; i++){
                if(word == list_projectiles[i]->pos_x){
                    if(list_projectiles[i]->used 
                    && list_projectiles[i]->curr_state == Projectile_state::before){ return true; }
                }
            }
            return false;

        case 5: // life
            return 2-life >= line;

        case 6: case 7: case 8: case 9: // scores
            int unit = word-6;
            if(curr_phase == Program_Phase::Wait_Player){ 
                return ((fake_cpu->high_score >> unit)&0x01) == line;
            }
            return ((fake_cpu->current_score >> unit)&0x01) == line;
    }
    return false;
}


void D_And_J_Climber_program::init_program(){
    beat = new Beat(MIN_TIME_WAIT, SPEED_INCREASE, SPEED_RANDOM_POWER
                    , SPEED_RANDOM_harmony, SPEED_LIMIT, SPEED_FALL_LIMIT);
    for(int i = 0; i < MAX_PROJECTILES; i++){
        list_projectiles[i] = new Projectile(beat
                                        , WAIT_BEFORE_MOVE_LIMIT
                                        , &nb_projectile_before, PLAYER_LOC
                                        , SPEED_INCREASE);
    }
    previous_input_action = false;
    previous_input_left = false; previous_input_right = false;
    init_phase();
}


void D_And_J_Climber_program::init_phase(){
    index_gen_table = 0;
    beat->init(0);
    change_wall();
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC[1]-1;
    curr_phase = Program_Phase::Wait_Player;
    fake_cpu->current_score = 0;
    life = 3;
    player_last_fall = 0;
}


void D_And_J_Climber_program::reset_fall_player(){
    player_last_fall = beat->player_speed_fall(int(index_gen_table/PLAYER_LOC[1]));
}

void D_And_J_Climber_program::check_fall_player(){
    if(player_last_fall == 0){
        if(!wall[pos_player[0]+pos_player[1]*PLAYER_LOC[0]] 
                && pos_player[1] < PLAYER_LOC[1]-1){
            pos_player[1]+=1;
        }
        reset_fall_player();
    }

    if(player_last_fall > 0){ player_last_fall -= 1;}
}



bool D_And_J_Climber_program::movement_on_platform(bool is_mov, int adding){
    if(wall[pos_player[0]+pos_player[1]*PLAYER_LOC[0]]
        || wall[adding + pos_player[0] + pos_player[1]*PLAYER_LOC[0]]
        || (is_mov && pos_player[1] >= PLAYER_LOC[1]-1)){ return true; }
    else {
        if(pos_player[1] < PLAYER_LOC[1]-1){
            pos_player[1]+=1; 
            reset_fall_player();
        }
    }
    return false;
}






void D_And_J_Climber_program::execute_program_loop(){
    switch (curr_phase) {
        case Program_Phase::Wait_Player:
            if(player_autorize_to_move(read_input_action())){
                curr_phase = Program_Phase::Play;
            }
            break;

        case Program_Phase::Play:
            if(player_autorize_to_move(read_input_action())){
                if(movement_on_platform(false)){
                    if(pos_player[1] == 0){ 
                        curr_phase = Program_Phase::Sucess; 
                        last_wait_x_counter = time_us_64_p(); x_counter = 0;
                    }
                    else { 
                        pos_player[1]-=1;
                        reset_fall_player();
                    }
                }
            }
            else if(player_autorize_to_move(read_input_left())){
                if(pos_player[0] > 0){
                    if(movement_on_platform(true, -1)){
                            pos_player[0]-=1;
                            reset_fall_player();
                    }
                }
            }
            else if(player_autorize_to_move(read_input_right())){
                if(pos_player[0] < PLAYER_LOC[0]-1){
                    if(movement_on_platform(true, 1)){
                        pos_player[0]+=1;
                        reset_fall_player();
                    }
                }
            }

            update_step();

            break;

        case Program_Phase::Sucess:
            if(last_wait_x_counter + WAIT_RETURN_AFTER_SUCESS < time_us_64_p()){
                change_wall();
                pos_player[0] = 1;
                pos_player[1] = PLAYER_LOC[1]-1;
                curr_phase = Program_Phase::Play;
            }
            break;

        case Program_Phase::Dead:
            break;
    }
}


void D_And_J_Climber_program::change_wall(){
    int index = 1;

    for(int i = 0; i < PLAYER_LOC[1] * PLAYER_LOC[0]; i++){ wall[i] = 0; }

    for(int i = 0; i < PLAYER_LOC[1]; i++){
        index = randrange(index_gen_table, max(0, index-1), min(PLAYER_LOC[0]-1, index+1));
        wall[index + i*PLAYER_LOC[0]] = 1;
        index_gen_table += 1;
    }

    beat->time_conf(int(index_gen_table/PLAYER_LOC[1]));
}


void D_And_J_Climber_program::stop_program_additional_work(){
    for(int i = 0; i < MAX_PROJECTILES; i++){
        delete list_projectiles[i];
        list_projectiles[i] = nullptr;
    }
}


bool D_And_J_Climber_program::read_input_action(){
    bool res = fake_cpu->get_input(0, 0);
    if(previous_input_action == false && res){
        previous_input_action = true;
        return true;
    } 
    previous_input_action = res;
    return false;
}

bool D_And_J_Climber_program::read_input_left(){
    bool res = fake_cpu->get_input(1, 0);
    if(previous_input_left == false && res){
        previous_input_left = true;
        return true;
    } 
    previous_input_left = res;
    return false;
}

bool D_And_J_Climber_program::read_input_right(){
    bool res = fake_cpu->get_input(1, 1);
    if(previous_input_right == false && res){
        previous_input_right = true;
        return true;
    } 
    previous_input_right = res;
    return false;
}


void D_And_J_Climber_program::additional_update_step(){
    check_fall_player();
}
