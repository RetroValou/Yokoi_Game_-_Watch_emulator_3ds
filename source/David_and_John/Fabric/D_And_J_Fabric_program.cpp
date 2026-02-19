#include "D_And_J_Fabric_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>



bool D_And_J_Fabric_program::input_start_game(){
    return read_input_action();
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
            int y = word/2; int x = word%2;
            int index = y+x*PLAYER_LOC_f[1];
            if((*list_projectiles)[index].used && (*list_projectiles)[index].curr_state == Projectile_state::down)
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

    for(size_t i = 0; i < PLAYER_LOC_f[1]; i++){
        (*list_projectiles)[i].pos_x = 0;
        (*list_projectiles)[i+PLAYER_LOC_f[1]].pos_x = 1;
    }

    index_gen_projectile = 1;
    id_score = 0;

    curr_wave = PROJECTILES_PER_WAVE_f;

    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    for(size_t i = 0; i < list_projectiles->size(); i++){ (*list_projectiles)[i].used = false; }
    init_blink();
}



void D_And_J_Fabric_program::adding_score_during_play(bool is_get){
    uint16_t score_to_add = is_get ? f_SCORE_GET : f_SCORE_SET;
    fake_cpu->play_sound(PIEZO_HIGHT, 150000, 1);
    fake_cpu->current_score += score_to_add;
}


void D_And_J_Fabric_program::adding_score(){
    // Score End wave
    uint16_t score_to_add = score_end;
    uint8_t nb_bip = 3;
    fake_cpu->play_sound(PIEZO_HIGHT, 150000, nb_bip);
    fake_cpu->current_score += score_to_add;
}


void D_And_J_Fabric_program::var_reset_gameplay(){
    nb_screen_clean += 1;
    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    curr_wave = PROJECTILES_PER_WAVE_f;
}


////// David pos estimation //////////////////////////////////////////////////////////////////
void D_And_J_Fabric_program::set_david_pos(uint8_t index){
    david_before[0] = (*list_projectiles)[index].pos_x;
    david_before[1] = index - david_before[0]*PLAYER_LOC_f[1];
}

void D_And_J_Fabric_program::stop_david(){
    david_before[0] = -1; david_before[1] = -1;
}

void D_And_J_Fabric_program::estimate_david_pos(){
    int index_min = -1;
    int value_max = 0;

    for(size_t i = 0; i < list_projectiles->size(); i++){
        uint8_t curr_i = i;
        if((*list_projectiles)[curr_i].used 
                && (*list_projectiles)[curr_i].curr_state == Projectile_state::down){
            if(value_max < (*list_projectiles)[curr_i].pos_y || index_min == -1){
                index_min = curr_i; 
                value_max = (*list_projectiles)[curr_i].pos_y;
            } 
        }
    }
    if(index_min == -1){ stop_david(); }
    else { set_david_pos(index_min); }
}


////// Projectiles ////////////////////////////////////////////////////////////////////

void D_And_J_Fabric_program::check_get_projectile(){
    uint8_t index = pos_player[1] + pos_player[0] * PLAYER_LOC_f[1]; 

    if(pos_player[0] == 0 && player_get_item == 0){ // check get
        if((*list_projectiles)[index].used
            && (*list_projectiles)[index].curr_state == Projectile_state::down){
            player_get_item = 1;
            adding_score_during_play(true);
            (*list_projectiles)[index].destroy();
            estimate_david_pos();
        }
    }

    else if(pos_player[0] == 1 && player_get_item == 1){ // check set
        if((*list_projectiles)[index].used
            && (*list_projectiles)[index].curr_state == Projectile_state::down){
            player_get_item = 0;
            adding_score_during_play(false);
            (*list_projectiles)[index].destroy();
            estimate_david_pos();
        }
    }

}


void D_And_J_Fabric_program::create_projectile(uint8_t x_choose, int nb_projectiles_activ){
    int i_choose = randrange(index_gen_projectile, 0, PLAYER_LOC_f[1] - nb_projectiles_activ -1);
    int index_free = get_index_projectile_not_use(i_choose, x_choose);
    (*list_projectiles)[index_free].start(index_gen_projectile, beat->new_index);

    (*list_projectiles)[index_free].pos_x = x_choose;
    int y_value = projectile_speed_fall(index_gen_projectile);
    y_value = PLAYER_LOC_f[1] - y_value;
    (*list_projectiles)[index_free].pos_y = y_value;

    index_gen_projectile += 1;
    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
}





void D_And_J_Fabric_program::update_projectile(){

    // Move projectiles
    bool moving_down = false;
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].used && (*list_projectiles)[i].flag_destroy){
            set_david_pos(i);
            (*list_projectiles)[i].destroy(); 
            curr_wave += count_projectile_active()+1;
            go_to_dead(-1);
            return;
        }
        moving_down = (*list_projectiles)[i].verify_move_down();
    }
    if(moving_down){ estimate_david_pos(); }
    
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
        if(curr_wave > 0){ curr_wave -= 1; }
        // First projectile
        create_projectile(0, nb_projectiles_activ);
        // Second projectile
        create_projectile(1, nb_projectiles_activ);
    }
}



void D_And_J_Fabric_program::additional_update_step(){
    update_projectile();
}


void D_And_J_Fabric_program::new_loop_gameplay(){
    go_to_play();
}


void D_And_J_Fabric_program::go_to_play(){
    stop_blink_player();
    stop_blink_last_life();
    curr_phase = Program_Phase::Play;
    player_get_item = 0;
    stop_david();
    // reset projectile for help player
    last_gen_projectile = projectile_creation_speed(index_gen_projectile);
    for(size_t i = 0; i < list_projectiles->size(); i++){
        (*list_projectiles)[i].used = false;
    }
}




void D_And_J_Fabric_program::play_logic(){
    if(player_autorize_to_move(read_input_action())){
        if(pos_player[0] == 0) { pos_player[0] = 1; }
        else { pos_player[0] = 0; }
        fake_cpu->play_sound(PIEZO_LITTLE_HIGHT, 150000, 1);
    }
    else if(player_autorize_to_move(read_input_up())){
        if(pos_player[1] > 0){
            pos_player[1] -= 1;
            fake_cpu->play_sound(PIEZO_LITTLE_HIGHT, 150000, 1);
        }
    }
    else if(player_autorize_to_move(read_input_down())){
        if(pos_player[1] < PLAYER_LOC_f[1]-1){
            pos_player[1] += 1;
            fake_cpu->play_sound(PIEZO_LITTLE_HIGHT, 150000, 1);
        }
    }

    check_get_projectile();
    update_step(); 
}

