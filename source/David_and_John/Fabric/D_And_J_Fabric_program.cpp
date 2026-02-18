#include "D_And_J_Fabric_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>



bool D_And_J_Fabric_program::input_start_game(){
    return read_input_up() || read_input_down();
}


bool D_And_J_Fabric_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // Dead
            if(curr_phase == Program_Phase::Dead){ return (word == david_before[0]); }
            return false;

        case 1: // player
        {
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            if(curr_phase == Program_Phase::Score_life_adding)
                { return (word == pos_player[0] + pos_player[1]*PLAYER_LOC_f[0] + player_get_item*PLAYER_LOC_f[0]*PLAYER_LOC_f[1]); }
            return (word == pos_player[0] + pos_player[1]*PLAYER_LOC_f[0] + player_get_item*PLAYER_LOC_f[0]*PLAYER_LOC_f[1]) 
                                && show_player;
        }        
        case 2: // projectile
        {
            int y = word/2;
            if((*list_projectiles)[y].used && (*list_projectiles)[y].curr_state == Projectile_state::down)
                { return true; }
            return false;
        }
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


int D_And_J_Fabric_program::count_projectile_active(){
    int nb = 0;
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].pos_x == 1 && (*list_projectiles)[i].used == true){
            nb += 1;
        }
    }
    return nb;
}

uint8_t D_And_J_Fabric_program::get_index_projectile_not_use(uint8_t i_nb, uint8_t x){
    int nb = 0;
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].pos_x == x
                && (*list_projectiles)[i].used == false){
            if(nb == i_nb){ return i; }
            nb += 1;
        }
    }
    return list_projectiles->size()-1;
}


uint64_t projectile_speed_fall(float x) {
    float result = f_SPEED_FALL_LIMIT[0] 
                + (f_SPEED_FALL_LIMIT[1] - f_SPEED_FALL_LIMIT[0])
                                            /(1+x*f_SPEED_FALL_increase);
    return static_cast<uint64_t>(std::max(f_SPEED_FALL_LIMIT[0], result));
}


void D_And_J_Fabric_program::init_phase(){
    nb_screen_clean = 1;
    beat->init(0);

    pos_player[1] = PLAYER_LOC_f[1]-1;
    pos_player[0] = 0;
    player_get_item = 0;

    david_before[0] = -1;
    david_before[1] = -1;

    curr_phase = Program_Phase::Wait_Player;
    fake_cpu->current_score = 0;
    life = 3;

    for(size_t i = 0; i < MAX_PROJECTILES_f; i++){
        (*list_projectiles)[i].pos_x = 0;
        (*list_projectiles)[i+MAX_PROJECTILES_f].pos_x = 1;
    }

    index_gen_projectile = 1;
    id_score = 0;

    curr_wave = PROJECTILES_PER_WAVE_f;

    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    for(size_t i = 0; i < list_projectiles->size(); i++){ (*list_projectiles)[i].used = false; }
    init_blink();
}




void D_And_J_Fabric_program::update_projectile(){

    // Move projectiles
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].used 
                && (*list_projectiles)[i].flag_destroy){
            (*list_projectiles)[i].destroy(); 
            curr_wave += count_projectile_active()*2;
            go_to_dead(-1);
            return;
        }
        (*list_projectiles)[i].verify_move_down();
    }
    

    // check if wave is finish
    bool not_active = true;
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].used){ not_active = false; break; }
    }

    if(curr_wave == 0 && not_active){ go_to_sucess(); return; }

    if(last_gen_projectile > 0) { last_gen_projectile -= 1; }

    int nb_projectiles_activ = count_projectile_active();

    // Generate new projectiles
    if(last_gen_projectile <= 0 && can_create_projectile() && beat->new_index
        && curr_wave > 0 && nb_projectiles_activ*2 < MAX_PROJECTILES_f)
    { 
        // First projectile
        int i_choose = randrange(index_gen_projectile, 0, nb_projectiles_activ-1);
        int index_free = get_index_projectile_not_use(i_choose, 0);
        (*list_projectiles)[index_free].start(index_gen_projectile, beat->new_index);
        (*list_projectiles)[index_free].pos_x = 0;
        (*list_projectiles)[index_free].pos_y = projectile_speed_fall(index_gen_projectile);
        index_gen_projectile += 1;
        last_gen_projectile = projectile_creation_speed(index_gen_projectile);
        if(curr_wave > 0){ curr_wave -= 1; }

        // Second projectile
        i_choose = randrange(index_gen_projectile, 0, nb_projectiles_activ-1);
        index_free = get_index_projectile_not_use(i_choose, 1);
        (*list_projectiles)[index_free].start(index_gen_projectile, beat->new_index);
        (*list_projectiles)[index_free].pos_x = 1;
        (*list_projectiles)[index_free].pos_y = projectile_speed_fall(index_gen_projectile);
        index_gen_projectile += 1;
        last_gen_projectile = projectile_creation_speed(index_gen_projectile);
        if(curr_wave > 0){ curr_wave -= 1; }
    }
}



void D_And_J_Fabric_program::additional_update_step(){
    update_projectile();
}


void D_And_J_Fabric_program::go_to_play(){
    stop_blink_player();
    stop_blink_last_life();
    curr_phase = Program_Phase::Play;
    // reset projectile for help player
    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    for(size_t i = 0; i < list_projectiles->size(); i++){
        (*list_projectiles)[i].used = false;
    }
}
