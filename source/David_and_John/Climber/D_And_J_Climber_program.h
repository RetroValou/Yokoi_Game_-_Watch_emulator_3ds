#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"

constexpr int PLAYER_LOC[2] = { 3, 4 };

constexpr uint8_t MIN_TIME_WAIT = 25;
constexpr float SPEED_INCREASE = 1.0f/30.0f;
constexpr float SPEED_RANDOM_POWER = 8.0f;
constexpr float SPEED_RANDOM_harmony = 1.2f;
constexpr float SPEED_LIMIT[2] = { 25.0f, 45.0f };
constexpr float SPEED_FALL_LIMIT[2] = { 40.0f, 70.0f };
constexpr float WAIT_BEFORE_MOVE_LIMIT[2] = { 30.0f, 80.0f };

constexpr float PROJECTILE_GEN_LIMIT[2] = { 0, 200 };
constexpr float PROJECTILE_GEN_INCREASE = 1.0f/40.0f;
constexpr float PROJECTILE_GEN_RANDOM_POWER = 15;
constexpr float PROJECTILE_GEN_RANDOM_HARMONY = 1.4f;

constexpr uint8_t MAX_PROJECTILES = 4;

constexpr uint64_t WAIT_RETURN_AFTER_SUCESS = (1000000);
constexpr uint64_t WAIT_RETURN_AFTER_LITTLE_DEAD = (1000000);
constexpr uint64_t WAIT_RETURN_AFTER_BIG_DEAD = (3000000);
constexpr uint64_t WAIT_ADDING_LIFE = (1000000);

constexpr uint8_t SCORE_END = 5;
constexpr uint8_t SCORE_BONUS_SPEED = 2;
constexpr uint8_t SCORE_BONUS_NO_FALL = 2;
constexpr uint16_t SPEED_BONUS_SPEED = 80;

constexpr uint32_t SCORE_ADD_LIFE[5] = {200, 500, 1000, 2000, 4000};
constexpr uint8_t SIZE_SCORE_ADD_LIFE = 5;



class David_And_John_fake_cpu;


class D_And_J_Climber_program: public David_And_John_program 
{
    public : 
        D_And_J_Climber_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu)
        {}

    private :
        uint8_t pos_player[2];
        bool wall[PLAYER_LOC[0]*PLAYER_LOC[1]];
        int index_gen_table;
        int nb_projectile_before;

        uint64_t last_gen_projectile = 0;
        Projectile* list_projectiles[MAX_PROJECTILES];
        uint32_t index_gen_projectile = 1;
        
        uint64_t player_last_fall = 0;
        uint64_t last_wait_x_counter = 0;

        uint8_t id_score;


        bool previous_input_action = false;
        bool previous_input_left = false;
        bool previous_input_right = false;

        bool is_falling;
        uint32_t last_speed_end;

    public :
        bool get_segments_state(uint8_t line, uint8_t word) override;
        void execute_program_loop() override;
        void init_program() override;

    private :
        void change_wall();
        void init_phase();
        void stop_program_additional_work() override;

        void reset_fall_player();
        void check_fall_player();
        bool movement_on_platform(bool is_mov = true, int adding = 0);

        bool read_input_action();
        bool read_input_left();
        bool read_input_right();

        void additional_update_step() override;

        void update_projectile();
        bool can_create_projectile(uint16_t nb_max = 1);

        void go_to_play();
        void play_logic();

        void go_to_dead(uint8_t id_projectile);
        void dead_logic();

        void go_to_sucess();
        void sucess_logic();
        void new_loop_gameplay();

        void go_to_win_life();
        void win_life_logic();
};