#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"


constexpr uint8_t PLAYER_LOC_f[2] = { 2, 5 };

constexpr uint8_t MAX_PROJECTILES_f = 10;
constexpr uint8_t PROJECTILES_PER_WAVE_f = 12;

constexpr float f_SPEED_LIMIT[2] = { 20.0f, 35.0f };
constexpr float f_WAIT_BEFORE_MOVE_LIMIT[2] = { 30.0f, 80.0f };

constexpr float f_SPEED_FALL_LIMIT[2] = { 6.0f, 25.0f };
constexpr float f_SPEED_FALL_increase = 1.0f/50.0f;
constexpr float f_SPEED_FALL_RANDOM_POWER = 1.0f;
constexpr float f_SPEED_FALL_RANDOM_HARMONY = 1.21f;

constexpr float f_PROJECTILE_GEN_LIMIT[2] = { 3, 100 };
constexpr float f_PROJECTILE_GEN_INCREASE = 1.0f/35.0f;
constexpr float f_PROJECTILE_GEN_RANDOM_POWER = 30;


class David_And_John_fake_cpu;



class D_And_J_Fabric_program: public David_And_John_program 
{
    public : 
        D_And_J_Fabric_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu, MAX_PROJECTILES_f, PLAYER_LOC_f, PLAYER_LOC_f)
        {
            R_projectile_gen_limit = f_PROJECTILE_GEN_LIMIT;
            R_projectile_gen_increase = f_PROJECTILE_GEN_INCREASE;
            R_projectile_gen_random_power = f_PROJECTILE_GEN_RANDOM_POWER;
            R_speed_limit = f_SPEED_LIMIT;
            R_speed_fall_limit = f_SPEED_FALL_LIMIT;
            R_wait_before_move_limit = f_WAIT_BEFORE_MOVE_LIMIT;
        }

    private :
        int pos_player[2];
        uint8_t player_get_item;

        int david_before[2];

        uint32_t nb_screen_clean;
        uint8_t curr_wave;

        uint64_t last_gen_projectile = 0;
        uint32_t index_gen_projectile = 1;
        
    public :
        bool get_segments_state(uint8_t line, uint8_t word) override;

    private :
        void init_phase() override;

        void additional_update_step() override;

        int count_projectile_active();
        uint8_t get_index_projectile_not_use(uint8_t i_nb, uint8_t x);
        void update_projectile();

        //void adding_score() override;
        //void var_reset_gameplay() override;
        void go_to_play() override;
        //void play_logic() override;

        //void new_loop_gameplay() override;

        bool input_start_game() override;
};