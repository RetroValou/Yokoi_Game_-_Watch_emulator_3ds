#pragma once
#include <cstdint>
#include <string>


double random(int t);
int randrange(int t, int a, int b);


class Projectile;



class Beat
{

    public : 
        Beat(const int v_min_time_wait
            , const float v_speed_increase
            , const float v_speed_random_power
            , const float v_speed_random_harmony
            , const float v_speed_limit[2]
            , const float v_speed_fall_limit[2]
        ) : 
            min_time_wait(v_min_time_wait),
            speed_increase(v_speed_increase),
            speed_random_power(v_speed_random_power),
            speed_random_harmony(v_speed_random_harmony)
            {
                speed_limit[0] = v_speed_limit[0]; speed_limit[1] = v_speed_limit[1];
                speed_fall_limit[0] = v_speed_fall_limit[0]; speed_fall_limit[1] = v_speed_fall_limit[1];
            }

    private :

        int time_wait_current;
        int time_wait_max;
        int min_time_wait;
        bool index_now;
        int new_index;
        int nb_bat;

        float speed_increase;
        float speed_random_power;
        float speed_random_harmony;
        float speed_limit[2];
        float speed_fall_limit[2];


    public : 
        float speed(float x);
        void init(int t_index);
        void time_conf(int t_index);
        bool update();
        bool move_ok(int index_test);
        uint64_t player_speed_fall(float x);
};



enum class Projectile_state {
    before,
    down,
    destroy
};



class Projectile
{

    public : 
        Projectile(Beat* v_beat
                , const float v_wait_before_move_limit[2]
                , int* v_nb_projectile_before
                , const int v_limit_pos[2]
                , const float v_speed_increase
            ) : 
            speed_increase(v_speed_increase)
            {
                beat = v_beat;
                wait_before_move_limit[0] = v_wait_before_move_limit[0]; wait_before_move_limit[1] = v_wait_before_move_limit[1];
                nb_projectile_before = v_nb_projectile_before;
                limit_pos[0] = v_limit_pos[0]; limit_pos[1] = v_limit_pos[1];
                used = false;
            }

    public :
        float pos_x;
        float pos_y;
        bool flag_destroy;
        bool used;
        Projectile_state curr_state;

    private :
        Beat* beat;

        int limit_pos[2];

        int index_beat;

        float speed_increase;

        float wait_before_move_limit[2];
        
        int* nb_projectile_before;

        

    public : 
        void start(int t, int v_index_beat);
        float wait(float x);
        void move_down();
        bool verify_move_down();
        bool is_same_pos(int p_pos_x, int p_pos_y);
        bool destroy();
        bool start_mode();
};
