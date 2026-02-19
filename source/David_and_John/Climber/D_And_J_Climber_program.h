#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"

constexpr uint8_t PLAYER_LOC[2] = { 3, 4 };
constexpr uint8_t MAX_PROJECTILES = 4;


class David_And_John_fake_cpu;


class D_And_J_Climber_program: public David_And_John_program 
{
    public : 
        D_And_J_Climber_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu, MAX_PROJECTILES, PLAYER_LOC, PLAYER_LOC)
        {}

    private :
        uint8_t pos_player[2];
        bool wall[PLAYER_LOC[0]*PLAYER_LOC[1]];
        int index_gen_table;

        uint64_t last_gen_projectile = 0;
        uint32_t index_gen_projectile = 1;
        
        uint64_t player_last_fall = 0;


        bool is_falling;
        uint32_t last_speed_end;

    public :
        bool get_segments_state(uint8_t line, uint8_t word) override;

    private :
        void change_wall();
        void init_phase() override;

        void reset_fall_player();
        void check_fall_player();
        bool movement_on_platform(bool is_mov = true, int adding = 0);
        void additional_update_step() override;

        void update_projectile();

        void adding_score() override;
        void var_reset_gameplay() override;
        void go_to_play() override;
        void play_logic() override;

        void new_loop_gameplay() override;

        bool input_start_game() override;
};