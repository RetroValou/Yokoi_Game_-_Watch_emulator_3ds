#include "D_And_J_Climber_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>




bool D_And_J_Climber_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            else if(curr_phase == Program_Phase::Sucess){ return (word == 12) && show_player; }
            else if(curr_phase == Program_Phase::Score_life_adding){ return (word == 12); }
            return (word == pos_player[0]+pos_player[1]*PLAYER_LOC[0]) && show_player;
        
        case 1: // wall
            return wall[word];

        case 2: // projectile
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x + (*list_projectiles)[i].pos_y*PLAYER_LOC[0]){
                    if((*list_projectiles)[i].used 
                    && (*list_projectiles)[i].curr_state == Projectile_state::down){ return true; }
                }
            }
            return false;

        case 3: // projectile wait before
            // if one of projectile is show before fall, not show wait before
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if((*list_projectiles)[i].used &&
                    (*list_projectiles)[i].curr_state == Projectile_state::before)
                { return false; }
            }
            return true;

        case 4: // projectile before
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x){
                    if((*list_projectiles)[i].used 
                    && (*list_projectiles)[i].curr_state == Projectile_state::before){ return true; }
                }
            }
            return false;

        case 5: // life
            return segment_life(word);

        case 6: case 7: case 8: case 9: // scores
            return segment_score(word, line-6);
    }
    return false;
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
    last_speed_end = 0;
    index_gen_projectile = 1;
    id_score = 0;
    last_gen_projectile = projectile_creation_speed(int(index_gen_table/PLAYER_LOC[1]));
    for(size_t i = 0; i < list_projectiles->size(); i++){ (*list_projectiles)[i].used = false; }
    init_blink();
}


void D_And_J_Climber_program::change_wall(){
    int index = 1;

    for(size_t i = 0; i < PLAYER_LOC[1] * PLAYER_LOC[0]; i++){ wall[i] = 0; }

    for(size_t i = 0; i < PLAYER_LOC[1]; i++){
        index = randrange(index_gen_table, max(0, index-1), min(PLAYER_LOC[0]-1, index+1));
        wall[index + i*PLAYER_LOC[0]] = 1;
        index_gen_table += 1;
    }

    beat->time_conf(int(index_gen_table/PLAYER_LOC[1]));
}


bool D_And_J_Climber_program::movement_on_platform(bool is_mov, int adding){
    if(wall[pos_player[0]+pos_player[1]*PLAYER_LOC[0]]
        || wall[adding + pos_player[0] + pos_player[1]*PLAYER_LOC[0]]
        || (is_mov && pos_player[1] >= PLAYER_LOC[1]-1)){ return true; }
    else {
        if(pos_player[1] < PLAYER_LOC[1]-1){
            pos_player[1]+=1; 
            is_falling = true;
            fake_cpu->play_sound(PIEZO_LOW);
            reset_fall_player();
        }
    }
    return false;
}



bool D_And_J_Climber_program::input_start_game(){
    return read_input_action();
}



void D_And_J_Climber_program::go_to_play(){
    stop_blink_player();
    stop_blink_last_life();
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC[1]-1;
    curr_phase = Program_Phase::Play;
}

void D_And_J_Climber_program::play_logic(){
    if(player_autorize_to_move(read_input_action())){
        if(movement_on_platform(false)){
            reset_fall_player(); 
        // Verify Projectile collider
        auto result = collider_all_projectiles(pos_player[0], pos_player[1]);
        if(result.first){ go_to_dead(result.second); }
        else{ 
                if(pos_player[1] == 0){ go_to_sucess(); }
                else { 
                    fake_cpu->play_sound(PIEZO_HIGHT);
                    pos_player[1]-=1; 
                }
            }
        }
    }
    else if(player_autorize_to_move(read_input_left())){
        if(pos_player[0] > 0){
            if(movement_on_platform(true, -1)){
                pos_player[0]-=1;
                fake_cpu->play_sound(PIEZO_LITTLE_HIGHT);
                reset_fall_player();
            }
        }
    }
    else if(player_autorize_to_move(read_input_right())){
        if(pos_player[0] < PLAYER_LOC[0]-1){
            if(movement_on_platform(true, 1)){
                pos_player[0]+=1;
                fake_cpu->play_sound(PIEZO_LITTLE_HIGHT);
                reset_fall_player();
            }
        }
    }
    update_step(); 
}



void D_And_J_Climber_program::new_loop_gameplay(){
    change_wall();
    go_to_play();
}

void D_And_J_Climber_program::adding_score(){
    uint16_t score_to_add = score_end;
    uint8_t nb_bip = 1;
    if(is_falling == false){
        score_to_add += score_bonus_no_fall;
        nb_bip += 1;
    }
    if(last_speed_end <= speed_for_bonus){
        score_to_add += score_bonus_speed;
        nb_bip += 1;
    }
    fake_cpu->play_sound(PIEZO_HIGHT, 150000, nb_bip);
    fake_cpu->current_score += score_to_add;
}

void D_And_J_Climber_program::var_reset_gameplay(){
    is_falling = false;
    last_speed_end = 0;
}


void D_And_J_Climber_program::additional_update_step(){
    check_fall_player();
    update_projectile();
    last_speed_end += 1;
    fake_cpu->set_accumulator_for_debug(last_speed_end > 255 ? 0xFF : last_speed_end);
}


/* PROJECTILES */
void D_And_J_Climber_program::update_projectile(){
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].flag_destroy){ (*list_projectiles)[i].destroy(); }

        bool same_pos = (*list_projectiles)[i].is_same_pos(pos_player[0], pos_player[1]);
        bool down = (*list_projectiles)[i].verify_move_down();
        if(same_pos && pos_player[1] != PLAYER_LOC[1]-1 && down){ 
            go_to_dead(i); return;
        }
    }

    if(last_gen_projectile > 0) { last_gen_projectile -= 1; }

    if(last_gen_projectile <= 0 && can_create_projectile() && beat->new_index)
    { 
        for(size_t i = 0; i < list_projectiles->size(); i++){
            if((*list_projectiles)[i].used == false){
                (*list_projectiles)[i].start(index_gen_projectile, beat->new_index);
                index_gen_projectile += 1;
                last_gen_projectile = projectile_creation_speed(int(index_gen_table/PLAYER_LOC[1]));
                break;
            }
        }
    }
}


/* FALL PLAYER */

void D_And_J_Climber_program::reset_fall_player(){
    player_last_fall = beat->player_speed_fall(int(index_gen_table/PLAYER_LOC[1]));
}

void D_And_J_Climber_program::check_fall_player(){
    if(player_last_fall == 0){
        if(!wall[pos_player[0]+pos_player[1]*PLAYER_LOC[0]] 
                && pos_player[1] < PLAYER_LOC[1]-1){
            pos_player[1]+=1;
            is_falling = true;
            //fake_cpu->play_sound(PIEZO_LOW);
        }
        reset_fall_player();
    }

    if(player_last_fall > 0){ player_last_fall -= 1;}
}

              



