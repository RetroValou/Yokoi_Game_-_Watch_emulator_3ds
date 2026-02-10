#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>
#include "../SM5XX/SM5XX.h"


constexpr uint64_t DEFAULT_SOUND_TIME = 120000;
constexpr uint64_t DEFAULT_NOTE = 8;
constexpr uint64_t WAIT_BETWEEN_BIP = 40000;


class David_And_John_program;

class David_And_John_fake_cpu: public SM5XX
{
    public : 
        David_And_John_fake_cpu() : 
            SM5XX("David_And_John\0")
            {}

    public :
        uint64_t high_score = 0;
        uint64_t current_score = 0;

    private : 
        David_And_John_program* curr_program;
        bool first_loop = true;
        bool show_all_segment = false;

        uint64_t wait_make_mark;
        bool is_during_wait_mark;

        // Note
        bool play_note;
        uint8_t sound_note;
        bool sound_curr_bit;
        uint8_t sound_curr_index;

        // size of note
        uint64_t sound_time_length;
        uint64_t sound_curr_time;

        uint8_t sound_nb_bip;
        uint8_t sound_curr_bip;
        bool sound_wait_silence;



        
    public:
        void init() override;
        void load_rom(const uint8_t* file_hex, size_t size_hex) override;

        bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override;
        
        // Save/Load state
        bool save_state(FILE* file) override;
        bool load_state(FILE* file) override;

        bool screen_is_on() override { return bp_lcd_blackplate; };
        uint8_t get_cpu_type_id() override { return 255; }

        void end_of_cpu() override;

        bool get_input(uint8_t col, uint8_t line);

        void play_sound(uint8_t note, uint64_t time_note = DEFAULT_SOUND_TIME, uint8_t nb_bip = 1);


    /// ##### FUNCTION ################################################# ///
    private:
        void execute_curr_opcode() override;

        void update_segment() override { };

        bool no_pc_increase(uint8_t opcode) override { return false; };
        bool is_on_double_octet(uint8_t opcode) override { return true; /* For lower speed of emulation */};

        bool condition_to_update_segment() override { return false; };

        void wake_up() override { };

        uint8_t read_rom_value() override { return 0x00; };
        uint8_t read_ram_value() override { return 0x00; };
        void write_ram_value(uint8_t value) override { };
        void set_ram_value(uint8_t col, uint8_t line, uint8_t value) override { };

        bool make_mark();

        bool get_active_sound() override;
        void update_sound() override;
        void update_note();


    public : // debug
        void set_beat_value_for_debug();
        void set_accumulator_for_debug(uint8_t value){ accumulator = value; }

};