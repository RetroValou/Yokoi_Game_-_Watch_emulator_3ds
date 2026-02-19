#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"

constexpr uint8_t PLAYER_LOC_A[2] = { 3, 5 };
constexpr uint8_t PROJECTILE_LOC[2] = { 1, 5 };

constexpr uint8_t MAX_PROJECTILES_A = 2;
constexpr float PROJECTILE_GEN_LIMIT[2] = { 40, 250 };

constexpr float POND_CHANGE_LIMIT[2] = { 40, 200 };
constexpr float POND_CHANGE_INCREASE = 1.0f/50.0f;
constexpr float POND_CHANGE_RANDOM_POWER = 0.4;
constexpr float POND_CHANGE_RANDOM_HARMONY = 1.4f;

constexpr float POND_DOWN_LIMIT[2] = { 5, 1 };
constexpr float POND_DOWN_INCREASE = 1/50;
constexpr float POND_DOWN_RANDOM_POWER = 0.4f;
constexpr float POND_DOWN_RANDOM_HARMONY = 1.4f;

constexpr uint8_t PLAYER_FALL = 20;




class David_And_John_fake_cpu;


class D_And_J_Avignon_program: public David_And_John_program 
{          
    public : 
        D_And_J_Avignon_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu, MAX_PROJECTILES_A, PROJECTILE_LOC, PLAYER_LOC_A)
        {
            R_projectile_gen_limit = PROJECTILE_GEN_LIMIT;
        }

    private :
        int pos_player[2];
        uint8_t pond_state[PLAYER_LOC_A[1]];

        uint64_t last_change_pond = 0;
        uint32_t index_pond = 0;
        uint64_t last_gen_projectile = 0;
        uint32_t index_gen_projectile = 1;
        uint32_t nb_screen_move;
        
        uint64_t player_last_fall = 0;
        bool is_falling;
        uint32_t last_speed_end;

    public :
        bool get_segments_state(uint8_t line, uint8_t word) override;

    private :
        void init_phase();

        void reset_fall_player();
        void check_fall_player();

        void additional_update_step() override;

        void update_projectile();
        void update_pond();

        void adding_score() override;
        void var_reset_gameplay() override;
        void go_to_play() override;
        void play_logic() override;

        void new_loop_gameplay() override;

        bool input_start_game() override;

};