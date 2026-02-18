#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"


constexpr uint8_t PLAYER_LOC_f[2] = { 2, 5 };

constexpr uint8_t MAX_PROJECTILES_f = 10;
constexpr uint8_t PROJECTILES_PER_WAVE_f = 12;



class David_And_John_fake_cpu;


class D_And_J_Fabric_program: public David_And_John_program 
{
    public : 
        D_And_J_Fabric_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu, MAX_PROJECTILES_f, PLAYER_LOC_f, PLAYER_LOC_f)
        {
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

        void update_projectile();

        void adding_score() override;
        void var_reset_gameplay() override;
        void go_to_play() override;
        void play_logic() override;

        void new_loop_gameplay() override;

        bool input_start_game() override;
};