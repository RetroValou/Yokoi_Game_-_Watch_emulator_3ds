#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include ".\common\game_basic_fct.h"

constexpr uint64_t WAIT_BEFORE_PLAYER_MOVE = (1000000/13); /* move player 13 fps -> Reproduce eink latency */
constexpr uint64_t WAIT_BEFORE_STEP = (1000000/60); /* Logic rythm to 60 fps */
constexpr uint64_t WAIT_BLINK = (160000);

constexpr uint8_t PIEZO_LOW = 16;//24;
constexpr uint8_t PIEZO_LITTLE_HIGHT = 13;//16;
constexpr uint8_t PIEZO_HIGHT = 12;//14;

constexpr uint64_t WAIT_RETURN_AFTER_SUCESS = (1000000);
constexpr uint64_t WAIT_RETURN_AFTER_LITTLE_DEAD = (1000000);
constexpr uint64_t WAIT_RETURN_AFTER_BIG_DEAD = (3000000);
constexpr uint64_t WAIT_ADDING_LIFE = (1000000);



constexpr uint8_t Default_MIN_TIME_WAIT = 25;
constexpr float Default_SPEED_INCREASE = 1.0f/30.0f;
constexpr float Default_SPEED_RANDOM_POWER = 8.0f;
constexpr float Default_SPEED_RANDOM_HARMONY = 1.2f;
constexpr float Default_SPEED_LIMIT[2] = { 25.0f, 45.0f };
constexpr float Default_SPEED_FALL_LIMIT[2] = { 40.0f, 70.0f };
constexpr float Default_WAIT_BEFORE_MOVE_LIMIT[2] = { 30.0f, 80.0f };

constexpr float Default_PROJECTILE_GEN_LIMIT[2] = { 0, 200 };
constexpr float Default_PROJECTILE_GEN_INCREASE = 1.0f/40.0f;
constexpr float Default_PROJECTILE_GEN_RANDOM_POWER = 15;
constexpr float Default_PROJECTILE_GEN_RANDOM_HARMONY = 1.4f;

constexpr uint8_t Default_SCORE_END = 5;
constexpr uint8_t Default_SCORE_BONUS_SPEED = 2;
constexpr uint8_t Default_SCORE_BONUS_NO_FALL = 2;
constexpr uint16_t Default_SPEED_BONUS_SPEED = 80;

constexpr uint32_t Default_SCORE_ADD_LIFE[5] = {200, 500, 1000, 2000, 4000};
constexpr uint8_t Default_SIZE_SCORE_ADD_LIFE = 5;




class David_And_John_fake_cpu;


enum class Program_Phase {
    Wait_Player,
    Play,
    Sucess,
    Dead,
    Score_life_adding
};

class David_And_John_program {

    public:
        virtual ~David_And_John_program() = default;
        explicit David_And_John_program(David_And_John_fake_cpu* v_fake_cpu
                                        , uint8_t v_max_projectile, const uint8_t v_limit_pos_projectile[2]
                                        , const uint8_t v_limit_pos_player[2]) :
            fake_cpu(v_fake_cpu),
            max_projectile(v_max_projectile)
        { 
            limit_pos_projectile[0] = v_limit_pos_projectile[0]; limit_pos_projectile[1] = v_limit_pos_projectile[1];
            limit_pos_player[0] = v_limit_pos_player[0]; limit_pos_player[1] = v_limit_pos_player[1];
        }

        virtual bool get_segments_state(uint8_t line, uint8_t word);
        void execute_program_loop();
        void stop_program();
        void init_program();

    public :
        Beat* beat;

    protected:
        David_And_John_fake_cpu* fake_cpu;
        Program_Phase curr_phase;

        int life;
        bool show_player;
        bool show_last_life;
        uint8_t limit_pos_player[2];

        uint8_t max_projectile;
        std::vector<Projectile>* list_projectiles;
        uint8_t limit_pos_projectile[2];

        uint64_t last_wait_x_counter = 0;
        uint8_t id_score;

    protected: // variable for rythm of game
        /* For beat rythm */
        uint8_t R_min_time_wait = Default_MIN_TIME_WAIT;
        float R_speed_increase = Default_SPEED_INCREASE;
        float R_speed_random_power = Default_SPEED_RANDOM_POWER;
        float R_speed_random_harmony = Default_SPEED_RANDOM_HARMONY;
        const float* R_speed_limit = Default_SPEED_LIMIT;
        const float* R_speed_fall_limit = Default_SPEED_FALL_LIMIT;
        const float* R_wait_before_move_limit = Default_WAIT_BEFORE_MOVE_LIMIT;

        /* For projectile rythm */
        const float* R_projectile_gen_limit = Default_PROJECTILE_GEN_LIMIT;
        float R_projectile_gen_increase = Default_PROJECTILE_GEN_INCREASE;
        float R_projectile_gen_random_power = Default_PROJECTILE_GEN_RANDOM_POWER;
        float R_projectile_gen_random_harmony = Default_PROJECTILE_GEN_RANDOM_HARMONY;

        /* For Scoring */
        uint8_t score_end = Default_SCORE_END;
        uint8_t score_bonus_end = Default_SCORE_BONUS_SPEED;
        uint8_t score_bonus_no_fall = Default_SCORE_BONUS_NO_FALL;
        uint8_t score_bonus_speed = Default_SCORE_BONUS_SPEED;
        uint8_t speed_for_bonus = Default_SPEED_BONUS_SPEED;
        const uint32_t* score_add_life = Default_SCORE_ADD_LIFE;
        uint8_t size_score_add_life = 5;

        
    protected:
        virtual void stop_program_additional_work(){ };
        bool player_autorize_to_move(bool input_result);

        void update_step();
        virtual void additional_update_step(){ };

        void init_blink(){ show_player = true; show_last_life = false; };
        void blink_player();
        void stop_blink_player();
        void blink_last_life();
        void stop_blink_last_life();

        bool segment_life(uint8_t word);
        bool segment_score(uint8_t word, uint8_t unit_score);


        bool read_input_action();
        bool read_input_left();
        bool read_input_right();
        bool read_input_up();
        bool read_input_down();

        virtual void init_phase(){};

        uint8_t id_score_sup(uint64_t score);
        uint64_t projectile_creation_speed(uint32_t x);
        bool can_create_projectile(uint16_t nb_max = 1);

        virtual void adding_score(){ };
        virtual void var_reset_gameplay(){};
        virtual void new_loop_gameplay(){ };
        virtual void go_to_play(){ };
        virtual void play_logic(){ };
        void go_to_win_life();
        void win_life_logic();
        void go_to_sucess();
        void sucess_logic();
        void go_to_dead(uint8_t id_projectile);
        void dead_logic();

        std::pair<bool, uint8_t> collider_all_projectiles(int x, int y);

        virtual bool input_start_game(){ return false; };

    private: 
        uint64_t last_time_player_move = 0;
        uint64_t last_time_step = 0;
        uint64_t last_wait_x_blink = 0;

        bool previous_input_action = false;
        bool previous_input_left = false;
        bool previous_input_right = false;
        bool previous_input_up = false;
        bool previous_input_down = false;

    private:
        void init_input();
};
 