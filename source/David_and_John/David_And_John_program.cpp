#include "David_And_John_program.h"
#include "std/timer.h"
#include "David_And_John_fake_cpu.h"
#include "./common/game_basic_fct.h"
#include <cmath>


bool David_And_John_program::player_autorize_to_move(bool input_result){
    if(input_result){
        uint64_t curr_time = time_us_64_p();
        if(last_time_player_move + WAIT_BEFORE_PLAYER_MOVE < curr_time
                || curr_time < last_time_player_move){ // for overflow
            last_time_player_move = curr_time;
            return true;
        }
    }
    return false;
}


void David_And_John_program::update_step(){
    uint64_t curr_time = time_us_64_p();
    if(last_time_step + WAIT_BEFORE_STEP < curr_time
            || curr_time < last_time_step){ // for overflow
        last_time_step = curr_time;
        beat->update();
        fake_cpu->set_beat_value_for_debug();
        additional_update_step();
    }
}



void David_And_John_program::blink_player(){
    if(last_wait_x_blink + WAIT_BLINK < time_us_64_p()  
        || last_wait_x_blink > time_us_64_p()){
        last_wait_x_blink = time_us_64_p();
        show_player = !show_player;
    }
}

void David_And_John_program::stop_blink_player(){
    show_player = true;
}


void David_And_John_program::blink_last_life(){
    if(last_wait_x_blink + WAIT_BLINK < time_us_64_p()  
        || last_wait_x_blink > time_us_64_p()){
        last_wait_x_blink = time_us_64_p();
        show_last_life = !show_last_life;
    }
}

void David_And_John_program::stop_blink_last_life(){
    show_last_life = false;
}



bool David_And_John_program::segment_life(uint8_t word) {
    return (2-life >= word) || (show_last_life && word == (3-life));
}


uint8_t get_digit(uint64_t number, uint8_t unit) {
    for (uint8_t i = 0; i < unit; i++) { number /= 10; }
    return number % 10;
}

bool David_And_John_program::segment_score(uint8_t word, uint8_t unit_score) {
    uint64_t value = fake_cpu->current_score;
    if(curr_phase == Program_Phase::Wait_Player){ value = fake_cpu->high_score; }
    return get_digit(value, unit_score) == word;
}



void David_And_John_program::init_program(){
    beat = new Beat(R_min_time_wait, R_speed_increase, R_speed_random_power
                    , R_speed_random_harmony, R_speed_limit, R_speed_fall_limit);

    list_projectiles = new std::vector<Projectile>();
    list_projectiles->clear();
    list_projectiles->reserve(max_projectile);

    for(int i = 0; i < max_projectile; i++){
        list_projectiles->emplace_back(beat
                                    , R_wait_before_move_limit
                                    , limit_pos_projectile
                                    , R_speed_increase);
    }
    last_time_step = 0,
    init_input();
    init_phase();
}


/* Adding LIFE if bigger than score */
uint8_t David_And_John_program::id_score_sup(uint64_t score){
    uint8_t i = 0;
    while(i < size_score_add_life){
        if(score < score_add_life[i]){ return i; }
        i+= 1;
    }
    return i;
}

/* PROJECTILES */
uint64_t David_And_John_program::projectile_creation_speed(uint32_t x){
    return static_cast<uint64_t>(
        R_projectile_gen_limit[0] + 
        (R_projectile_gen_limit[1] - R_projectile_gen_limit[0])/(1+x*R_projectile_gen_increase) 
        + R_projectile_gen_random_power * std::sin(x * R_projectile_gen_random_harmony)
    ); 
}

bool David_And_John_program::can_create_projectile(uint16_t nb_max){
    uint16_t nb_projectile_before = 0;
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].used == true
                && (*list_projectiles)[i].curr_state == Projectile_state::before){
            nb_projectile_before += 1;
        }
    }

    return nb_projectile_before < nb_max;
}

std::pair<bool, uint8_t> David_And_John_program::collider_all_projectiles(int x, int y){
    for(size_t i = 0; i < list_projectiles->size(); i++){
        if((*list_projectiles)[i].is_same_pos(x, y)){ return  {true, i}; }
    }
    return {false, 0};
}


/* INPUT */
void David_And_John_program::init_input(){
    previous_input_action = false;
    previous_input_left = false; previous_input_right = false;
    previous_input_up = false; previous_input_down = false;
}


bool David_And_John_program::read_input_action(){
    bool res = fake_cpu->get_input(0, 0);
    if(previous_input_action == false && res){
        previous_input_action = true;
        return true;
    } 
    previous_input_action = res;
    return false;
}

bool David_And_John_program::read_input_left(){
    bool res = fake_cpu->get_input(1, 0);
    if(previous_input_left == false && res){
        previous_input_left = true;
        return true;
    } 
    previous_input_left = res;
    return false;
}

bool David_And_John_program::read_input_right(){
    bool res = fake_cpu->get_input(1, 1);
    if(previous_input_right == false && res){
        previous_input_right = true;
        return true;
    } 
    previous_input_right = res;
    return false;
}

bool David_And_John_program::read_input_up(){
    bool res = fake_cpu->get_input(2, 0);
    if(previous_input_up == false && res){
        previous_input_up = true;
        return true;
    } 
    previous_input_up = res;
    return false;
}

bool David_And_John_program::read_input_down(){
    bool res = fake_cpu->get_input(2, 1);
    if(previous_input_down == false && res){
        previous_input_down = true;
        return true;
    } 
    previous_input_down = res;
    return false;
}


void David_And_John_program::stop_program(){ 
    delete list_projectiles; list_projectiles = nullptr;
    delete beat; beat = nullptr; 
    stop_program_additional_work(); 
}


/* LOGIC */

void David_And_John_program::go_to_sucess(){
    curr_phase = Program_Phase::Sucess; 
    last_wait_x_counter = time_us_64_p();
    adding_score();
    var_reset_gameplay();
}


void David_And_John_program::sucess_logic(){
    blink_player();
    if(last_wait_x_counter + WAIT_RETURN_AFTER_SUCESS < time_us_64_p()){
        uint8_t new_score_id = id_score_sup(fake_cpu->current_score);
        if(life != 3 && new_score_id != id_score){ go_to_win_life(); id_score = new_score_id; }
        else { new_loop_gameplay(); }
    }
}




void David_And_John_program::go_to_win_life(){
    curr_phase = Program_Phase::Score_life_adding; 
    last_wait_x_counter = time_us_64_p();
    fake_cpu->play_sound(PIEZO_LITTLE_HIGHT, 100000, 3);
    life += 1;
}


void David_And_John_program::win_life_logic(){
    blink_last_life();
    if(last_wait_x_counter + WAIT_ADDING_LIFE < time_us_64_p()){
        new_loop_gameplay();
    }
}


void David_And_John_program::go_to_dead(uint8_t id_projectile){
    (*list_projectiles)[id_projectile].used = false;
    curr_phase = Program_Phase::Dead;
    last_wait_x_counter = time_us_64_p();
    life -= 1;
    if(life > 0){ fake_cpu->play_sound(PIEZO_LOW, 150000, 2); }
    else{ fake_cpu->play_sound(PIEZO_LOW, 150000, 5); }
}

void David_And_John_program::dead_logic(){
    //blink_player();
    if(life > 0){ // Little dead -> we have life
        if(last_wait_x_counter + WAIT_RETURN_AFTER_LITTLE_DEAD < time_us_64_p()){
            go_to_play();
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



void David_And_John_program::execute_program_loop(){
    switch (curr_phase) {
        case Program_Phase::Wait_Player:
            if(input_start_game()){
                curr_phase = Program_Phase::Play;
                fake_cpu->play_sound(PIEZO_HIGHT);
            }
            break;

        case Program_Phase::Play: 
            play_logic(); break;

        case Program_Phase::Sucess:
            sucess_logic(); break;

        case Program_Phase::Dead:
            dead_logic(); break;

        case Program_Phase::Score_life_adding:
            win_life_logic(); break;
            break;
    }
}
