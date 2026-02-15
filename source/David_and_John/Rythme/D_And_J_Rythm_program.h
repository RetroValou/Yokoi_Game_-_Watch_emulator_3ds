#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"


constexpr uint8_t PLAYER_LOC_ry[2] = { 3, 1 };
constexpr uint8_t PROJECTILE_LOC_ry[2] = { 3, 3 };
constexpr uint8_t MAX_PROJECTILES_ry = 5;
constexpr uint8_t PROJECTILES_PER_WAVE = 15;
constexpr uint8_t DODO_TIME_DAVID = 40;

constexpr uint8_t ry_SCORE_BONUS_multi = 3;
constexpr uint8_t ry_SCORE_BONUS_destroy = 2;

constexpr float ry_WAIT_BEFORE_MOVE_LIMIT[2] = { 35.0f, 80.0f };

constexpr float ry_PROJECTILE_GEN_LIMIT[2] = { -20, 130 };
constexpr float ry_PROJECTILE_GEN_INCREASE = 1.0f/50.0f;
constexpr float ry_PROJECTILE_GEN_RANDOM_POWER = 20;


constexpr float ry_NB_SIMULT_PROJECTILE_LIMIT[2] = { 4, -1 };
constexpr float ry_NB_SIMULT_PROJECTILE_MAX_MIN[2] = { 1, 3 };
constexpr float ry_NB_SIMULT_PROJECTILE_INCREASE = 1.0f/60.0f;
constexpr float ry_NB_SIMULT_PROJECTILE_RANDOM_POWER = 0.3f;



class David_And_John_fake_cpu;


class D_And_J_Rythm_program: public David_And_John_program 
{
    public : 
        D_And_J_Rythm_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu, MAX_PROJECTILES_ry, PROJECTILE_LOC_ry, PLAYER_LOC_ry)
        {
            R_wait_before_move_limit = ry_WAIT_BEFORE_MOVE_LIMIT;
            R_projectile_gen_limit = ry_PROJECTILE_GEN_LIMIT;
            R_projectile_gen_increase = ry_NB_SIMULT_PROJECTILE_INCREASE;
        }

    private :
        uint8_t pos_player_x;
        uint8_t dodo_david;
        uint8_t last_dodo;

        bool tuyau_state[PROJECTILE_LOC_ry[0]];

        uint32_t nb_screen_clean;
        uint8_t curr_wave;

        uint64_t last_gen_projectile = 0;
        uint32_t index_gen_projectile = 1;
        

    public :
        bool get_segments_state(uint8_t line, uint8_t word) override;

    private :
        void init_phase() override;
        void additional_update_step() override;

        void update_projectile();

        void adding_score() override;
        void adding_score_during_play(uint8_t nb_destroy);
        void var_reset_gameplay() override;
        void go_to_play() override;
        void play_logic() override;

        void new_loop_gameplay() override;

        bool input_start_game() override;
};