#include <cmath>
#include <algorithm>
#include <cstdint>
#include <string>
#include <algorithm>

#include "./game_basic_fct.h"


double random(int t) {
    return std::fmod(std::fmod(std::sin(t), 1.0) + 1.0, 1.0);
}

int randrange(int t, int a, int b) {
    if (a > b)
        std::swap(a, b);

    int value = static_cast<int>(random(t) * (b - a + 1) + a);
    return std::max(std::min(value, b), a);
}


void Beat::init(int t_index){
    time_wait_current = 0;
    time_wait_max = 0;
    time_conf(t_index);
    index_now = 0;
    time_wait_current = time_wait_max;
    new_index = true;
}

void Beat::time_conf(int t_index){
    time_wait_current -= time_wait_max;
    nb_bat = (int)(std::max(speed(t_index)/min_time_wait, 1.0f));
    time_wait_max = (int)(std::max(int(speed(t_index)/ nb_bat), min_time_wait));
    time_wait_current += time_wait_max;
}       

bool Beat::update(){
    new_index = false;
    time_wait_current -= 1;
    if(time_wait_current <= 0){
        time_wait_current = time_wait_max;
        index_now += 1;
        if(index_now >= nb_bat){ index_now = 0; }
        new_index = true;
    }
    return new_index;
}

bool Beat::move_ok(int index_test){
    return new_index && (
        (index_test == index_now)
        || (index_test >= nb_bat && index_now == 0)
    );
}


float Beat::speed(float x) {
    return speed_limit[0] 
            + (speed_limit[1] - speed_limit[0])/(1+x*speed_increase) 
            + speed_random_power * sin(x * speed_random_harmony);
}



uint64_t Beat::player_speed_fall(float x) {
    float result = speed_fall_limit[0] 
                + (speed_fall_limit[1] - speed_fall_limit[0])
                                            /(1+x*speed_increase);
    return static_cast<uint64_t>(std::max(speed_fall_limit[0], result));
}



void Projectile::start(int t, int v_index_beat){
    (*nb_projectile_before) += 1;
    pos_y = 0; pos_x = randrange(t, 0, limit_pos[0] - 1);
    flag_destroy = false;
    curr_state = Projectile_state::before;
    index_beat = v_index_beat;
    used = true;
}


float Projectile::wait(float x){
    float result = wait_before_move_limit[0] 
        + (wait_before_move_limit[1] - wait_before_move_limit[0])
                                            /(1+x*speed_increase);
    return std::max(wait_before_move_limit[0], result);
}


void Projectile::move_down(){
    pos_y += 1;
    if(pos_y >= limit_pos[1]){
        flag_destroy = true;
        pos_y -= 1;          
    }
}

bool Projectile::verify_move_down(){
    if(!(beat->move_ok(index_beat))){ return false; }
    else if(curr_state == Projectile_state::before){
        (*nb_projectile_before) -= 1;
        curr_state = Projectile_state::down;
        return true;
    }
    else if(curr_state == Projectile_state::down){ move_down(); return true; }
    else if(curr_state == Projectile_state::destroy){ return true; }
    return false;
}


bool Projectile::is_same_pos(int p_pos_x, int p_pos_y){
    return (p_pos_x == pos_x) && (p_pos_y == pos_y) 
            && (curr_state == Projectile_state::down)
            && (used);
}
        
bool Projectile::destroy(){
    if(curr_state != Projectile_state::destroy){ 
        curr_state = Projectile_state::destroy;
        used = false;
        return true;
    }
    return false;
}


bool Projectile::start_mode(){ return (curr_state == Projectile_state::before); }
    
