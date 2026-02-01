#pragma once

#include "David_and_John/David_And_John_program.h"
#include "David_and_John/David_And_John_fake_cpu.h"

constexpr uint8_t PLAYER_LOC_X = 3;
constexpr uint8_t PLAYER_LOC_Y = 4;

class David_And_John_fake_cpu;


class D_And_J_Climber_program: public David_And_John_program 
{
    public : 
        D_And_J_Climber_program(David_And_John_fake_cpu* v_fake_cpu) : 
            David_And_John_program(v_fake_cpu)
        {}

        bool get_segments_state(uint8_t line, uint8_t word) override;
        void execute_program_loop() override;
        void init_program() override;

    private :
        uint8_t pos_player[2];
        bool wall[PLAYER_LOC_X*PLAYER_LOC_Y];
};