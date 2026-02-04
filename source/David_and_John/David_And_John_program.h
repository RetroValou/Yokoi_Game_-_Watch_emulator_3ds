#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>
#include ".\common\game_basic_fct.h"

constexpr uint64_t WAIT_BEFORE_PLAYER_MOVE = (1000000/15);
constexpr uint64_t WAIT_BEFORE_STEP = (1000000/15);


class David_And_John_fake_cpu;


enum class Program_Phase {
    Wait_Player,
    Play,
    Sucess,
    Dead
};

class David_And_John_program {

    public:
        explicit David_And_John_program(David_And_John_fake_cpu* v_fake_cpu) :
            fake_cpu(v_fake_cpu)
        { }

        virtual bool get_segments_state(uint8_t line, uint8_t word);
        virtual void execute_program_loop();
        virtual void init_program();
        virtual void stop_program(){ delete beat; beat = nullptr; stop_program_additional_work(); }

    protected:
        David_And_John_fake_cpu* fake_cpu;
        Beat* beat;
        Program_Phase curr_phase;
        
        virtual void stop_program_additional_work(){ }
        bool player_autorize_to_move(bool input_result);
        void update_step();
        virtual void additional_update_step(){ }


    private: 
        uint64_t last_time_player_move = 0;
        uint64_t last_time_step = 0;

};
 