#include "D_And_J_Rythm_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>



bool D_And_J_Rythm_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            if(curr_phase == Program_Phase::Score_life_adding){ return (word == pos_player_x); }
            return (word == pos_player_x) && show_player;
        
        case 1: case 2: case 3: case 14: // decord(button + line)
            return true;

        case 4: // David wet -> dead
            if(curr_phase == Program_Phase::Dead){ return true; }
            return false;

        case 5: // life
            return segment_life(word);

        case 6: case 7: case 8: case 9: // scores
            return segment_score(word, line-6);

        case 10: // David Dodo
            if(curr_phase == Program_Phase::Dead){ return false; }
            if(curr_phase == Program_Phase::Score_life_adding){ return true; }
            return true /*&& show_player*/;

        case 11: // David ZZZ
            if(curr_phase == Program_Phase::Dead){ return false; }
            return (dodo_david == word);

        case 12: // Projectile
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if(word == (*list_projectiles)[i].pos_x + (*list_projectiles)[i].pos_y*PROJECTILE_LOC_ry[0]*2){
                    if((*list_projectiles)[i].used 
                        && (*list_projectiles)[i].curr_state == Projectile_state::down){ return true; }
                }
            }
            return false;

        case 13: // tuyau move
        {
            uint8_t ind = word%PROJECTILE_LOC_ry[0];
            return ((tuyau_state[ind] && word >= PROJECTILE_LOC_ry[0])  // white
                || (!tuyau_state[ind] && word < PROJECTILE_LOC_ry[0])); // black
        }
    }
    return false;
}





void D_And_J_Rythm_program::init_phase(){
    nb_screen_clean = 1;
    beat->init(0);
    pos_player_x = PLAYER_LOC_ry[1]-1;
    curr_phase = Program_Phase::Wait_Player;
    fake_cpu->current_score = 0;
    life = 3;

    index_gen_projectile = 1;
    id_score = 0;

    dodo_david = 0;
    last_dodo = DODO_TIME_DAVID;

    curr_wave = PROJECTILES_PER_WAVE;

    for(int i = 0; i < PROJECTILE_LOC_ry[0] ; i++){ tuyau_state[i] = false; }

    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    for(size_t i = 0; i < list_projectiles->size(); i++){ (*list_projectiles)[i].used = false; }
    init_blink();
}



/* Score */

void D_And_J_Rythm_program::adding_score_during_play(uint8_t nb_destroy){
    if(nb_destroy == 0){ return; }
    uint16_t score_to_add = ry_SCORE_BONUS_destroy;
    uint8_t nb_bip = 1;
    for(int j = 1; j < nb_destroy; j++){
        nb_bip += 1;
        score_to_add += ry_SCORE_BONUS_multi;
    }
    fake_cpu->play_sound(PIEZO_HIGHT, 150000, nb_bip);
    fake_cpu->current_score += score_to_add;
}

void D_And_J_Rythm_program::adding_score(){
    // Score End wave
    uint16_t score_to_add = score_end;
    uint8_t nb_bip = 3;
    fake_cpu->play_sound(PIEZO_HIGHT, 150000, nb_bip);
    fake_cpu->current_score += score_to_add;
}


/* PROJECTILES */

uint64_t nb_projectile_simultanee_ry(uint32_t x){
    float calcul = ry_NB_SIMULT_PROJECTILE_LIMIT[0] + 
        (ry_NB_SIMULT_PROJECTILE_LIMIT[1] - ry_NB_SIMULT_PROJECTILE_LIMIT[0])/(1+x*ry_NB_SIMULT_PROJECTILE_INCREASE) 
        + ry_NB_SIMULT_PROJECTILE_RANDOM_POWER * std::sin(x * Default_PROJECTILE_GEN_RANDOM_HARMONY);

    if(calcul < ry_NB_SIMULT_PROJECTILE_MAX_MIN[0]){ return ry_NB_SIMULT_PROJECTILE_MAX_MIN[0]; }
    if(calcul >= ry_NB_SIMULT_PROJECTILE_MAX_MIN[1]){ return ry_NB_SIMULT_PROJECTILE_MAX_MIN[1]; }
    return static_cast<uint64_t>(calcul); 
}


bool color_projectile(uint32_t x){
    int calcul = randrange((x*3.0f)+1.0f, 0, 1);
    return (calcul == 1);
}



void D_And_J_Rythm_program::update_projectile(){

    uint8_t nb_destroy = 0;
    // Move projectiles
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].used 
            && (*list_projectiles)[i].flag_destroy){
            // dead if at the end, color of projectile (storage in pos_x) 
            //      is not the same to color of tuyau
            bool color = (*list_projectiles)[i].pos_x >= PROJECTILE_LOC_ry[0];
            uint8_t ind =  (*list_projectiles)[i].pos_x%PROJECTILE_LOC_ry[0];
            if(tuyau_state[ind] != color){ 
                go_to_dead(i); curr_wave += 1; /* not validate wave */ return; // stop job
            }
            (*list_projectiles)[i].destroy(); 
            nb_destroy += 1;
        }
        (*list_projectiles)[i].verify_move_down();
    }

    adding_score_during_play(nb_destroy);
    

    // check if wave is finish
    bool not_active = true;
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].used){ not_active = false; break; }
    }

    if(curr_wave == 0 && not_active){ go_to_sucess(); return; }

    if(last_gen_projectile > 0) { last_gen_projectile -= 1; }

    // Generate new projectiles
    if(last_gen_projectile <= 0 && can_create_projectile() && beat->new_index
        && curr_wave > 0)
    { 
        uint8_t nb_create = nb_projectile_simultanee_ry(index_gen_projectile);
        uint8_t x_proj = 0;

        // First projectile
        for(size_t i = 0; i < list_projectiles->size(); i++){
            if((*list_projectiles)[i].used == false){
                (*list_projectiles)[i].start(index_gen_projectile, beat->new_index);
                index_gen_projectile += 1;
                last_gen_projectile = projectile_creation_speed(index_gen_projectile);
                x_proj = (*list_projectiles)[i].pos_x;
                if(color_projectile(index_gen_projectile)){
                    (*list_projectiles)[i].pos_x += PROJECTILE_LOC_ry[0]; 
                }
                curr_wave -= 1;
                break;
            }
        }

        if(curr_wave < nb_create){ nb_create = curr_wave; }

        // Additional projectile (in simultanee) 
        for(size_t j = 1; j < nb_create; j++){
            x_proj = (x_proj+1)%PLAYER_LOC_ry[0]; // force pos x to right
            for(size_t i = 0; i < list_projectiles->size(); i++){
                if((*list_projectiles)[i].used == false){
                    (*list_projectiles)[i].start(index_gen_projectile, beat->new_index);
                    (*list_projectiles)[i].pos_x = x_proj;
                    if(color_projectile(index_gen_projectile)){
                        (*list_projectiles)[i].pos_x += PROJECTILE_LOC_ry[0]; 
                    }
                    index_gen_projectile += 1;
                    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
                    curr_wave -= 1;
                    break;
                }
            }
        }
    }
}







void D_And_J_Rythm_program::additional_update_step(){
    
    update_projectile();

    last_dodo -= 1;
    if(last_dodo == 0){
        last_dodo = DODO_TIME_DAVID;
        dodo_david = (dodo_david+1)%3;
    }
}



void D_And_J_Rythm_program::var_reset_gameplay(){
    nb_screen_clean += 1;
    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    curr_wave = PROJECTILES_PER_WAVE;
}


bool D_And_J_Rythm_program::input_start_game(){
    return read_input_action();
}

void D_And_J_Rythm_program::new_loop_gameplay(){
    go_to_play();
}


void D_And_J_Rythm_program::play_logic(){
    if(player_autorize_to_move(read_input_right())){
        if(pos_player_x < PLAYER_LOC_ry[0]-1){
            pos_player_x += 1; 
            fake_cpu->play_sound(PIEZO_LITTLE_HIGHT); 
        }
    }
    else if(player_autorize_to_move(read_input_left())){
        if(pos_player_x > 0){
            pos_player_x -= 1; 
            fake_cpu->play_sound(PIEZO_LITTLE_HIGHT); 
        }
    }
    else if(player_autorize_to_move(read_input_action())){
        tuyau_state[pos_player_x] = !tuyau_state[pos_player_x];
        if(tuyau_state[pos_player_x]){ fake_cpu->play_sound(PIEZO_HIGHT); }
        else{ fake_cpu->play_sound(PIEZO_LOW); }
    }
    update_step(); 
}


void D_And_J_Rythm_program::go_to_play(){
    stop_blink_player();
    stop_blink_last_life();
    curr_phase = Program_Phase::Play;
    // reset projectile for help player
    for(size_t i = 0; i < list_projectiles->size(); i++){
        (*list_projectiles)[i].used = false;
    }
}
