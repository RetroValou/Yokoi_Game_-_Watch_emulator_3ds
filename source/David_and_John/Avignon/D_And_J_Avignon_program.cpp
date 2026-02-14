
#include "D_And_J_Avignon_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>




uint64_t Pond_changement_speed(uint32_t x){
    return static_cast<uint64_t>(
        POND_CHANGE_LIMIT[0] + 
        (POND_CHANGE_LIMIT[1] - POND_CHANGE_LIMIT[0])/(1+x*POND_CHANGE_INCREASE) 
        + POND_CHANGE_RANDOM_POWER * std::sin(x * POND_CHANGE_RANDOM_HARMONY)
    );
    
}

uint8_t Pond_nb_down(uint32_t x){
    return static_cast<uint8_t>(
        POND_DOWN_LIMIT[0] + 
        (POND_DOWN_LIMIT[1] - POND_DOWN_LIMIT[0])/(1+x*POND_DOWN_INCREASE) 
        + POND_DOWN_RANDOM_POWER * std::sin(x * POND_DOWN_RANDOM_HARMONY)
    );

}


bool D_And_J_Avignon_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
        {
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            else if(curr_phase == Program_Phase::Sucess){ return (word == 15) && show_player; }
            else if(curr_phase == Program_Phase::Score_life_adding){ return (word == 15); }
            return (word == pos_player[0]+pos_player[1]*PLAYER_LOC_A[0]) && show_player;
        }

        case 1: // pond
        {
            uint8_t index = word/3;
            uint8_t state = word%3;
            if(index < PLAYER_LOC_A[1]){ return (pond_state[index] == state); }
            return (index == PLAYER_LOC_A[1] && state == 0);   
        }
        case 2: // projectile
        {
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x + (*list_projectiles)[i].pos_y*PROJECTILE_LOC[0]){
                    if((*list_projectiles)[i].used 
                        && (*list_projectiles)[i].curr_state == Projectile_state::down){ return true; }
                }
            }
            return false;
        }

        case 3: // projectile wait before
        {
            if(curr_phase == Program_Phase::Sucess){ return false; }
            // if one of projectile is show before fall, not show wait before
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if((*list_projectiles)[i].used &&
                    (*list_projectiles)[i].curr_state == Projectile_state::before)
                { return false; }
            }
            return true;
        }
        case 4: // projectile before
        {
            if(curr_phase == Program_Phase::Sucess){ return false; }
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x){
                    if((*list_projectiles)[i].used 
                    && (*list_projectiles)[i].curr_state == Projectile_state::before){ return true; }
                }
            }
            return false;
        }
        case 5: // life
        {
            return segment_life(word);
        }
        case 6: case 7: case 8: case 9: // scores
        {
            return segment_score(word, line-6);
        }
        case 10: // canon, just background
        {
            return true;
        }
    }
    return false;
}




void D_And_J_Avignon_program::init_phase(){
    nb_screen_move = 0;
    beat->init(0);
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC_A[1]-1;
    curr_phase = Program_Phase::Wait_Player;
    fake_cpu->current_score = 0;
    life = 3;
    for(int i = 0; i < PLAYER_LOC_A[1]; i++){ pond_state[i] = 2; }
    player_last_fall = 0;
    last_speed_end = 0;
    index_gen_projectile = 1;
    id_score = 0;
    index_pond = 2;
    last_gen_projectile = projectile_creation_speed(nb_screen_move);
    last_change_pond = 10; // first on manual for show of player pond fall
    for(size_t i = 0; i < list_projectiles->size(); i++){ (*list_projectiles)[i].used = false; }
    init_blink();
}





void D_And_J_Avignon_program::go_to_play(){
    stop_blink_player();
    stop_blink_last_life();
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC_A[1]-1;
    curr_phase = Program_Phase::Play;
}



/* PROJECTILES */

void D_And_J_Avignon_program::update_projectile(){
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].flag_destroy){ (*list_projectiles)[i].destroy(); }

        bool same_pos = (*list_projectiles)[i].is_same_pos(pos_player[0], pos_player[1]);
        bool down = (*list_projectiles)[i].verify_move_down();
        if(same_pos && pos_player[1] != PLAYER_LOC_A[1]-1 && down){ 
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
                last_gen_projectile = projectile_creation_speed(nb_screen_move);
                break;
            }
        }
    }
}


void D_And_J_Avignon_program::update_pond(){
    // Destroy pond with "fragility"
    if(beat->new_index){
        for(int i = 0; i < PLAYER_LOC_A[1]; i++){
            if(pond_state[i] == 1){ pond_state[i] = 0; }
        }
    }

    // Diminution for change pond
    if(last_change_pond > 0) { last_change_pond -= 1; }

    if(last_change_pond <= 0 && beat->new_index)
    {
        // Calculate nb pond is downd
        uint8_t nb_pond_down = 0;
        for(int i = 0; i < PLAYER_LOC_A[1]; i++){ if(pond_state[i] == 0){ nb_pond_down+= 1; } }
        
        // Estimate if destroy platform or reconstruct this
        bool mode_destroy = false;
        uint8_t i_tmp = 0;
        if(nb_pond_down < Pond_nb_down(nb_screen_move)){
            mode_destroy = true;
            i_tmp = randrange(index_pond, 0, PLAYER_LOC_A[1]-2-nb_pond_down);
        }
        else { i_tmp = randrange(index_pond, 0, nb_pond_down); }

        // Match index with platform we want (not destroy or destroy)
        int i_pond = 0;
        uint8_t j = 0;
        for(j = 0; j < PLAYER_LOC_A[1]-1; j++){ // Never impact first platform
            if((mode_destroy && pond_state[j] == 2) 
                || (!mode_destroy && pond_state[j] == 0)){
                    i_pond += 1;
                    if(i_pond == i_tmp){ break; }
                }
        }
        if(j == PLAYER_LOC_A[1]-1){ j-=1; }
        i_pond = j;
        // Apply change pond
        if(pond_state[i_pond] == 2){ pond_state[i_pond] = 1; }
        else{ pond_state[i_pond] = 2; }
        // Reset variables
        index_pond += 1;
        last_change_pond = Pond_changement_speed(nb_screen_move);
    }
}



void D_And_J_Avignon_program::play_logic(){
    if(player_autorize_to_move(read_input_action())){
        if(pos_player[0] == 1){
            pos_player[0] = 0;
            is_falling = true;
            fake_cpu->play_sound(PIEZO_HIGHT);
            reset_fall_player();
        }
        else if(pos_player[0] == 0) {
            auto result = collider_all_projectiles(pos_player[0], pos_player[1]);
            if(result.first){ go_to_dead(result.second); }
            else{ 
                pos_player[0] += 1;
                pos_player[1] -= 1;
                fake_cpu->play_sound(PIEZO_LOW);
            }
        }
    }
    else if(player_autorize_to_move(read_input_left())){
        if(pos_player[0] == 0){ // On sky
            auto result = collider_all_projectiles(pos_player[0], pos_player[1]+1);
            if(result.first){ go_to_dead(result.second); }
            else{ 
                pos_player[0] += 1; 
                fake_cpu->play_sound(PIEZO_LITTLE_HIGHT); 
            }
        }
        else if(pos_player[0] == 1){ // On ground
            if(pos_player[1] < PLAYER_LOC_A[1]-1){
                pos_player[1] += 1; 
                fake_cpu->play_sound(PIEZO_LITTLE_HIGHT);
                if(pond_state[pos_player[1]] == 0){
                    pos_player[0] = 2; go_to_dead(max_projectile);
                }
            }
        }
    }
    else if(player_autorize_to_move(read_input_right())){
        if(pos_player[0] == 0){ // On sky
            auto result = collider_all_projectiles(pos_player[0], pos_player[1]);
            if(result.first){ go_to_dead(result.second); }
            else{ 
                pos_player[0] += 1; 
                pos_player[1] -= 1;
                fake_cpu->play_sound(PIEZO_LITTLE_HIGHT); 
            }
        }
        else if(pos_player[0] == 1){ // On ground
            if(pond_state[pos_player[1]] == 0){
                pos_player[0] = 2; go_to_dead(max_projectile);
            }
            else { 
                pos_player[1] -= 1; 
                fake_cpu->play_sound(PIEZO_LITTLE_HIGHT);
            }
        }
        
    }
    if(pos_player[1] < 0){ go_to_sucess(); }
    update_step(); 
}


bool D_And_J_Avignon_program::input_start_game(){
    return read_input_action();
}

void D_And_J_Avignon_program::new_loop_gameplay(){
    go_to_play();
}

void D_And_J_Avignon_program::adding_score(){
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


/* FALL PLAYER */

void D_And_J_Avignon_program::var_reset_gameplay(){
    is_falling = false;
    last_speed_end = 0;
    nb_screen_move += 1;
}

void D_And_J_Avignon_program::reset_fall_player(){
    player_last_fall = PLAYER_FALL;
}

void D_And_J_Avignon_program::check_fall_player(){
    if(player_last_fall == 0){
        reset_fall_player();
        if(pos_player[0] == 0){
            bool validate_up = true;
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if((*list_projectiles)[i].is_same_pos(pos_player[0], pos_player[1])){ 
                    validate_up = false; go_to_dead(i); 
                }
            }
            if(validate_up){
                pos_player[0] += 1;
                pos_player[1] -= 1;
            }
        }
    }
    if(player_last_fall > 0){ player_last_fall -= 1;}
}



void D_And_J_Avignon_program::additional_update_step(){
    update_projectile();
    update_pond();
    check_fall_player();
    last_speed_end += 1;
    fake_cpu->set_accumulator_for_debug(last_speed_end > 255 ? 0xFF : last_speed_end);
}


