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

constexpr uint8_t MAX_PROJECTILES = 4;

constexpr uint64_t WAIT_RETURN_AFTER_SUCESS = (1000000);


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
        int life;
        Projectile* list_projectiles[MAX_PROJECTILES];

        uint64_t player_last_fall = 0;
        uint64_t last_wait_x_counter = 0;
        uint16_t x_counter = 0;


        bool previous_input_action = false;
        bool previous_input_left = false;
        bool previous_input_right = false;

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

    
};