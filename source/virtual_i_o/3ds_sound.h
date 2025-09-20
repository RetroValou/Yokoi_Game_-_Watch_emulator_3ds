#pragma once

#include "SM5XX/SM5XX.h"
#include <cstdint>
#include <3ds.h>

constexpr uint8_t NB_BUFFER = 5;
constexpr uint8_t SEQUENCE_SECURITY_ADD = 4;

class Virtual_Sound {
    private :
        int16_t* buffer_sound[NB_BUFFER];
        ndspWaveBuf waveBuf[NB_BUFFER];
        size_t curr_size_buffer[NB_BUFFER];

        uint8_t curr_buffer = 0;
        uint16_t curr_sequence = 0;
        uint16_t length_max_sequence;
        uint16_t curr_wait_before_update = 0;

        bool curr_value;
        uint32_t accu_freq_sequence = 0;
        float fps_screen = 0;
        uint32_t base_freq;
        uint16_t divide_freq;
        
    public : 
        void configure_sound();
        void initialize(uint32_t v_freq, uint16_t v_divide_freq, float fps_screen);
        void play_sample();
        void update_sound(SM5XX* cpu);
        void Quit_Game();
        void Exit();

    private : 
        //void lissage_sound();

};



