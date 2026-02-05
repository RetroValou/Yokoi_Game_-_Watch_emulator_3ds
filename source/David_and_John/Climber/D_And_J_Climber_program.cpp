#include "D_And_J_Climber_program.h"
#include <cstdint>
#include <string>
#include "David_and_John/common/game_basic_fct.h"
#include "std/timer.h"
#include <cmath>



/* Adding LIFE if bigger than score */
uint8_t id_score_sup(uint64_t score){
    uint8_t i = 0;
    while(i < SIZE_SCORE_ADD_LIFE){
        if(score < SCORE_ADD_LIFE[i]){ return i; }
        i+= 1;
    }
    return i;
}


uint64_t projectile_creation_speed(uint32_t x){
    return static_cast<uint64_t>(
        PROJECTILE_GEN_LIMIT[0] + 
        (PROJECTILE_GEN_LIMIT[1] - PROJECTILE_GEN_LIMIT[0])/(1+x*PROJECTILE_GEN_INCREASE) 
        + PROJECTILE_GEN_RANDOM_POWER * std::sin(x * PROJECTILE_GEN_RANDOM_HARMONY)
    );
}

uint8_t get_digit(uint64_t number, uint8_t unit) {
    for (uint8_t i = 0; i < unit; i++) { number /= 10; }
    return number % 10;
}


bool D_And_J_Climber_program::get_segments_state(uint8_t line, uint8_t word){
    switch (line)
    {
        case 0: // player
            if(curr_phase == Program_Phase::Wait_Player){ return false; }
            else if(curr_phase == Program_Phase::Sucess){ return (word == 12) && show_player; }
            return (word == pos_player[0]+pos_player[1]*PLAYER_LOC[0]) && show_player;
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
            return (2-life >= word) || (show_last_life && word == (2-life-1));

        case 6: case 7: case 8: case 9: // scores
            uint8_t unit = line-6;
            uint64_t value = fake_cpu->current_score;
            if(curr_phase == Program_Phase::Wait_Player){ value = fake_cpu->high_score; }
            return get_digit(value, unit) == word;
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
    last_speed_end = 0;
    index_gen_projectile = 1;
    id_score = 0;
    last_gen_projectile = projectile_creation_speed(int(index_gen_table/PLAYER_LOC[1]));
    nb_projectile_before = 0;
    for(int i = 0; i < MAX_PROJECTILES; i++){ list_projectiles[i]->used = false; }
    init_blink();
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




void D_And_J_Climber_program::execute_program_loop(){
    switch (curr_phase) {
        case Program_Phase::Wait_Player:
            if(player_autorize_to_move(read_input_action())){
                curr_phase = Program_Phase::Play;
                fake_cpu->play_sound(PIEZO_HIGHT);
            }
            break;

        case Program_Phase::Play:
            if(player_autorize_to_move(read_input_action())){
                if(movement_on_platform(false)){
                    reset_fall_player(); 
                    // Verify Projectile collider
                    bool validate_up = true;
                    for(int i = 0; i < MAX_PROJECTILES; i++){
                        if(list_projectiles[i]->is_same_pos(pos_player[0], pos_player[1])){ 
                            validate_up = false; go_to_dead(i); 
                        }
                    }
                    if(validate_up){
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
            update_step(); break;


        case Program_Phase::Sucess:
            sucess_logic(); break;

        case Program_Phase::Dead:
            dead_logic(); break;

        case Program_Phase::Score_life_adding:
            win_life_logic(); break;
            break;
    }
}

void D_And_J_Climber_program::dead_logic(){
    //blink_player();
    if(life > 0){ // Little dead -> we have life
        if(last_wait_x_counter + WAIT_RETURN_AFTER_LITTLE_DEAD < time_us_64_p()){
            stop_blink_player();
            pos_player[0] = 1;
            pos_player[1] = PLAYER_LOC[1]-1;
            curr_phase = Program_Phase::Play;
        }
    }
    else {  // Big dead -> life = 0 -> return to select screen
        if(last_wait_x_counter + WAIT_RETURN_AFTER_BIG_DEAD < time_us_64_p()){
            stop_blink_player();
            if(fake_cpu->current_score > fake_cpu->high_score){
                fake_cpu->high_score = fake_cpu->current_score;
            }
            init_phase();
        }
    }

}

void D_And_J_Climber_program::go_to_dead(uint8_t id_projectile){
    list_projectiles[id_projectile]->used = false;
    curr_phase = Program_Phase::Dead;
    last_wait_x_counter = time_us_64_p();
    life -= 1;
    if(life > 0){ fake_cpu->play_sound(PIEZO_LOW, 150000, 2); }
    else{ fake_cpu->play_sound(PIEZO_LOW, 150000, 5); }
}


void D_And_J_Climber_program::new_loop_gameplay(){
    change_wall();
    stop_blink_player();
    pos_player[0] = 1;
    pos_player[1] = PLAYER_LOC[1]-1;
    curr_phase = Program_Phase::Play;
}

void D_And_J_Climber_program::go_to_sucess(){
    curr_phase = Program_Phase::Sucess; 
    last_wait_x_counter = time_us_64_p();
    uint16_t score_to_add = SCORE_END;
    uint8_t nb_bip = 1;
    if(is_falling == false){
        score_to_add += SCORE_BONUS_NO_FALL;
        nb_bip += 1;
    }
    if(last_speed_end <= SPEED_BONUS_SPEED){
        score_to_add += SCORE_BONUS_SPEED;
        nb_bip += 1;
    }
    fake_cpu->play_sound(PIEZO_HIGHT, 150000, nb_bip);
    fake_cpu->current_score += score_to_add;
    is_falling = false;
    last_speed_end = 0;
}

void D_And_J_Climber_program::sucess_logic(){
    blink_player();
    if(last_wait_x_counter + WAIT_RETURN_AFTER_SUCESS < time_us_64_p()){
        uint8_t new_score_id = id_score_sup(fake_cpu->current_score);
        if(life != 3 && new_score_id != id_score){ go_to_win_life(); id_score = new_score_id; }
        else { new_loop_gameplay(); }
    }
}




void D_And_J_Climber_program::go_to_win_life(){
    curr_phase = Program_Phase::Score_life_adding; 
    last_wait_x_counter = time_us_64_p();
    fake_cpu->play_sound(PIEZO_LITTLE_HIGHT, 100000, 3);
    life += 1;
}


void D_And_J_Climber_program::win_life_logic(){
    blink_last_life();
    if(last_wait_x_counter + WAIT_ADDING_LIFE < time_us_64_p()){
        stop_blink_last_life();
        new_loop_gameplay();
    }
}



void D_And_J_Climber_program::additional_update_step(){
    check_fall_player();
    update_projectile();
    last_speed_end += 1;
    fake_cpu->set_accumulator_for_debug(last_speed_end > 255 ? 0xFF : last_speed_end);
}


/* PROJECTILES */

bool D_And_J_Climber_program::can_create_projectile(uint16_t nb_max){
    return nb_projectile_before < nb_max;
}

void D_And_J_Climber_program::update_projectile(){
    for(int i = 0; i < MAX_PROJECTILES; i++){
        if(list_projectiles[i]->flag_destroy){ list_projectiles[i]->destroy(); }

        bool same_pos = list_projectiles[i]->is_same_pos(pos_player[0], pos_player[1]);
        bool down = list_projectiles[i]->verify_move_down();
        if(same_pos && pos_player[1] != PLAYER_LOC[1]-1 && down){ 
            go_to_dead(i); return;
        }
    }

    if(last_gen_projectile > 0) { last_gen_projectile -= 1; }

    if(last_gen_projectile <= 0 && can_create_projectile() && beat->new_index)
    { 
        for(int i = 0; i < MAX_PROJECTILES; i++){
            if(list_projectiles[i]->used == false){
                list_projectiles[i]->start(index_gen_projectile, beat->new_index);
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

              



void D_And_J_Climber_program::stop_program_additional_work(){
    for(int i = 0; i < MAX_PROJECTILES; i++){
        delete list_projectiles[i];
        list_projectiles[i] = nullptr;
    }
}


/* INPUT */
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

