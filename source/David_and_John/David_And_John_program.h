#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>
#include ".\common\game_basic_fct.h"

constexpr uint64_t WAIT_BEFORE_PLAYER_MOVE = (1000000/13); /* move player 13 fps -> Reproduce eink latency */
constexpr uint64_t WAIT_BEFORE_STEP = (1000000/60); /* Logic rythm to 60 fps */
constexpr uint64_t WAIT_BLINK = (160000);

constexpr uint8_t PIEZO_LOW = 16;//24;
constexpr uint8_t PIEZO_LITTLE_HIGHT = 13;//16;
constexpr uint8_t PIEZO_HIGHT = 12;//14;


class David_And_John_fake_cpu;


enum class Program_Phase {
    Wait_Player,
    Play,
    Sucess,
    Dead,
    Score_life_adding
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
    public :
        Beat* beat;

    protected:
        David_And_John_fake_cpu* fake_cpu;
        Program_Phase curr_phase;
        int life;
        bool show_player;
        bool show_last_life;

        virtual void stop_program_additional_work(){ }
        bool player_autorize_to_move(bool input_result);
        void update_step();
        virtual void additional_update_step(){ }
        void init_blink(){ show_player = true; show_last_life = false; };
        void blink_player();
        void stop_blink_player();
        void blink_last_life();
        void stop_blink_last_life();

        bool segment_life(uint8_t word);
        bool segment_score(uint8_t word, uint8_t unit_score);



    private: 
        uint64_t last_time_player_move = 0;
        uint64_t last_time_step = 0;
        uint64_t last_wait_x_blink = 0;

};
 