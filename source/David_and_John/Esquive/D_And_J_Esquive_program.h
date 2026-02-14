#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"


constexpr uint8_t PLAYER_LOC_e[2] = { 3, 4 };
constexpr uint8_t MAX_PROJECTILES_e = 7;


constexpr float E_PROJECTILE_GEN_LIMIT[2] = { 0, 120 };
constexpr float E_SPEED_LIMIT[2] = { 5.0f, 40.0f };

constexpr float E_NB_SIMULT_PROJECTILE_LIMIT[2] = { 4, -1 };
constexpr float E_NB_SIMULT_PROJECTILE_MAX_MIN[2] = { 1, 2 };
constexpr float E_NB_SIMULT_PROJECTILE_INCREASE = 1/50;
constexpr float E_NB_SIMULT_PROJECTILE_RANDOM_POWER = 1.1f;

constexpr uint8_t WARNING_TIME_DAVID = 20;



class David_And_John_fake_cpu;


class D_And_J_Esquive_program: public David_And_John_program 
{
    public : 
        D_And_J_Esquive_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu, MAX_PROJECTILES_e, PLAYER_LOC_e, PLAYER_LOC_e)
        {
            R_projectile_gen_limit = E_PROJECTILE_GEN_LIMIT;
            R_speed_limit = E_SPEED_LIMIT;
        }

    private :
        int pos_player[2];
        uint8_t pos_David;
        bool warning_david;
        uint8_t last_warning;

        uint32_t nb_screen_move;

        uint64_t last_gen_projectile = 0;
        uint32_t index_gen_projectile = 1;
        
        bool is_falling;
        uint32_t last_speed_end;
        uint64_t player_last_fall = 0;

    public :
        bool get_segments_state(uint8_t line, uint8_t word) override;

    private :
        void init_phase() override;

        void additional_update_step() override;

        void update_projectile();

        void adding_score() override;
        void var_reset_gameplay() override;
        void go_to_play() override;
        void play_logic() override;

        void new_loop_gameplay() override;

        bool input_start_game() override;
};