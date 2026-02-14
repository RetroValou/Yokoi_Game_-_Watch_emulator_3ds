#include "D_And_J_Esquive_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>



bool D_And_J_Esquive_program::input_start_game(){
    return read_input_left() || read_input_right();
}


uint8_t david_new_pos(uint32_t nb_sc){
    return static_cast<uint8_t>(randrange(nb_sc*1.2f+6, 0, PLAYER_LOC_e[0]-1));
}


uint64_t nb_projectile_simultanee(uint32_t x){
    float calcul = E_NB_SIMULT_PROJECTILE_LIMIT[0] + 
        (E_NB_SIMULT_PROJECTILE_LIMIT[1] - E_NB_SIMULT_PROJECTILE_LIMIT[0])/(1+x*E_NB_SIMULT_PROJECTILE_INCREASE) 
        + E_NB_SIMULT_PROJECTILE_RANDOM_POWER * std::sin(x * Default_PROJECTILE_GEN_RANDOM_HARMONY);

    if(calcul < E_NB_SIMULT_PROJECTILE_MAX_MIN[0]){ return E_NB_SIMULT_PROJECTILE_MAX_MIN[0]; }
    if(calcul >= E_NB_SIMULT_PROJECTILE_MAX_MIN[1]){ return E_NB_SIMULT_PROJECTILE_MAX_MIN[1]; }
    return static_cast<uint64_t>(calcul); 
}




bool D_And_J_Esquive_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            else if(curr_phase == Program_Phase::Sucess){ return (word == 12) && show_player; }
            else if(curr_phase == Program_Phase::Score_life_adding){ return (word == 12); }
            return (word == pos_player[0]+pos_player[1]*PLAYER_LOC_e[0]) && show_player;
        
        case 1: // David
            if(curr_phase == Program_Phase::Sucess){ return false;}
            else if(curr_phase == Program_Phase::Score_life_adding){ return false;}

            return (pos_David == word);

        case 2: // projectile
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x + (*list_projectiles)[i].pos_y*PLAYER_LOC_e[0]){
                    if((*list_projectiles)[i].used 
                    && (*list_projectiles)[i].curr_state == Projectile_state::down){ return true; }
                }
            }
            return false;

        case 3: // Help of David
            if(curr_phase == Program_Phase::Sucess){ return false;}
            else if(curr_phase == Program_Phase::Score_life_adding){ return false;}
            else if(curr_phase == Program_Phase::Wait_Player){ return false;}

            if(warning_david){ return ((pos_David + PLAYER_LOC_e[0]) == word); }
            else{ return (pos_David == word); }

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

        case 10: // decors barre
            return true;
    }
    return false;
}



void D_And_J_Esquive_program::init_phase(){
    nb_screen_move = 1;
    beat->init(0);
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC_e[1]-1;
    curr_phase = Program_Phase::Wait_Player;
    fake_cpu->current_score = 0;
    life = 3;

    player_last_fall = 0;
    last_speed_end = 0;
    index_gen_projectile = 1;
    id_score = 0;

    warning_david = false;
    last_warning = WARNING_TIME_DAVID;
    pos_David = david_new_pos(nb_screen_move);

    last_gen_projectile = projectile_creation_speed(nb_screen_move);
    for(size_t i = 0; i < list_projectiles->size(); i++){ (*list_projectiles)[i].used = false; }
    init_blink();
}


void D_And_J_Esquive_program::new_loop_gameplay(){
    pos_David = david_new_pos(nb_screen_move);
    go_to_play();
}


void D_And_J_Esquive_program::adding_score(){
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


/* PROJECTILES */
void D_And_J_Esquive_program::update_projectile(){
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].flag_destroy){ (*list_projectiles)[i].destroy(); }

        bool same_pos = (*list_projectiles)[i].is_same_pos(pos_player[0], pos_player[1]);
        bool down = (*list_projectiles)[i].verify_move_down();
        if(same_pos && pos_player[1] != PLAYER_LOC_e[1]-1 && down){ 
            go_to_dead(i); return;
        }
    }

    if(last_gen_projectile > 0) { last_gen_projectile -= 1; }

    if(last_gen_projectile <= 0 && can_create_projectile() && beat->new_index)
    { 
        uint8_t nb_create = nb_projectile_simultanee(index_gen_projectile);
        uint8_t x_proj = 0;
        for(size_t i = 0; i < list_projectiles->size(); i++){
            if((*list_projectiles)[i].used == false){
                (*list_projectiles)[i].start(index_gen_projectile, beat->new_index);
                index_gen_projectile += 1;
                last_gen_projectile = projectile_creation_speed(nb_screen_move);
                x_proj = (*list_projectiles)[i].pos_x;
                break;
            }
        }
        // Adding other generate -> In generaly only other one 
        for(size_t j = 1; j < nb_create; j++){
            x_proj = (x_proj+1)%PLAYER_LOC_e[0]; // force pos x to right
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if((*list_projectiles)[i].used == false){
                    (*list_projectiles)[i].start(index_gen_projectile, beat->new_index);
                    index_gen_projectile += 1;
                    last_gen_projectile = projectile_creation_speed(nb_screen_move);
                    (*list_projectiles)[i].pos_x = x_proj;
                    break;
                }
            }
        }
    }
}



void D_And_J_Esquive_program::var_reset_gameplay(){
    is_falling = false;
    last_speed_end = 0;
    nb_screen_move += 1;
}


void D_And_J_Esquive_program::additional_update_step(){
    
    last_speed_end += 1;
    update_projectile();

    last_warning -= 1;
    if(last_warning == 0){
        last_warning = WARNING_TIME_DAVID;
        warning_david = !warning_david;
    }

    fake_cpu->set_accumulator_for_debug(last_speed_end > 255 ? 0xFF : last_speed_end);
}


void D_And_J_Esquive_program::play_logic(){
    if(player_autorize_to_move(read_input_left())){
        if(pos_player[1] < PLAYER_LOC_e[1]-1){
            auto result = collider_all_projectiles(pos_player[0], pos_player[1]+1);
            if(result.first){ go_to_dead(result.second); }
            else{ 
                pos_player[1] += 1; 
                fake_cpu->play_sound(PIEZO_HIGHT); 
                is_falling = true;
            }
        }
    }
    else if(player_autorize_to_move(read_input_right())){
        auto result = collider_all_projectiles(pos_player[0], pos_player[1]);
        if(result.first){ go_to_dead(result.second); }
        else{ 
            if( (pos_player[1] > 0) || (pos_David == pos_player[0]) ){
                pos_player[1] -= 1; 
                fake_cpu->play_sound(PIEZO_HIGHT); 
            }
        }        
    }
    else if(player_autorize_to_move(read_input_up())){
        if(pos_player[0] > 0){
            pos_player[0] -= 1;
            fake_cpu->play_sound(PIEZO_LITTLE_HIGHT); 
        }
    }
    else if(player_autorize_to_move(read_input_down())){
        if(pos_player[0] < PLAYER_LOC_e[0]-1){
            pos_player[0] += 1;
            fake_cpu->play_sound(PIEZO_LITTLE_HIGHT); 
        }
    }

    if(pos_player[1] < 0){ go_to_sucess(); }
    update_step(); 
}


void D_And_J_Esquive_program::go_to_play(){
    stop_blink_player();
    stop_blink_last_life();
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC_e[1]-1;
    curr_phase = Program_Phase::Play;
}
