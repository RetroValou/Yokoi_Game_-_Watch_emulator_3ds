#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>
#include "SM5XX/SM5XX.h"


constexpr uint8_t SM511_2_RAM_COL = 8;
constexpr uint8_t SM511_2_RAM_LINE = 16;
constexpr uint8_t SM511_2_RAM_VIDEO_COL = 3; // 2 for SM511 - 3 for SM512 -> Just visual of col ram 3 not exist for SM511

constexpr uint8_t SM511_2_ROM_COL = 4;
constexpr uint8_t SM511_2_ROM_LINE = 16;
// constexpr int ROM_WORD = 63; declared in SM5xx.h

constexpr uint16_t SM511_2_MELODY_ROM_SIZE = 256;


constexpr uint8_t SM511_2_ROM_MAME_LINE = 16; // per each col : 11 standars lines + 5 fake lines (0x00...) = 16 lines
constexpr uint8_t SM511_2_ROM_MAME_WORD = 63 + 1; // per each line : 63 standars word + 1 fake word (0x80) = 64 word

//  constexpr int FREQUENCY_CPU = 32768; declared in SM5xx.h
constexpr uint32_t SM511_2_TIME_CHANGE_NOTE = 62500; // in u_second = 62.5 ms


struct Note_Melody {
    uint8_t note;
    bool double_time;
    bool octave;
};


class SM511_2: public SM5XX 
{
public : 
    SM511_2() : SM5XX("SM511_SM512\0") {}

public:
    void init() override;
    void load_rom(const uint8_t* file_hex, size_t size_hex) override;
    void load_rom_melody(const uint8_t* file_hex, size_t size_hex) override;

    bool screen_is_on() override;
    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override;
    bool get_active_sound() override;

    // Save/Load state
    bool save_state(FILE* file) override;
    bool load_state(FILE* file) override;
    uint8_t get_cpu_type_id() override { return 2; } // CPU_TYPE_SM511_2

private:
    /// Variables / register /// 
    uint8_t ram[SM511_2_RAM_COL][SM511_2_RAM_LINE];
    uint8_t rom[SM511_2_ROM_COL][SM511_2_ROM_LINE][ROM_WORD];

    uint8_t segment_on[SM511_2_RAM_VIDEO_COL+1][SM511_2_RAM_LINE]; // +1 for bs output  

    ProgramCounter r_buffer_program_counter;  // second buffer of program counter (buffer of S buffer)
    
    uint8_t rom_melody[256];
    uint8_t rom_melody_address; 

    // input
    uint8_t w_shift_register; // Used for input -> Sequential
    uint8_t s_pin; // pin used for input sequential, not direcly send by w in SM511/SM512 (not like SM510)

    bool bc_lcd_stop;

    uint8_t l_bs; // connected to bs1 -> blink output
    uint8_t x_bs; // connected to bs2 -> blink output
    uint8_t y_bs; 

    uint8_t alternativ_col_ram;

    // Melody
    bool me_melody_activate;
    bool mes_melody_finish;
    uint16_t melody_cycle_count;
    uint16_t nb_cycle_need_for_change_note;
    Note_Melody curr_note_melody;

    uint8_t curr_phase;
    uint8_t cycle_in_curr_phase;

/// ##### FUNCTION ################################################# ///
private:
    void update_segment() override;
    void update_sound() override;

    bool no_pc_increase(uint8_t opcode) override;
    bool is_on_double_octet(uint8_t opcode) override;

    void execute_curr_opcode() override;

    bool condition_to_update_segment() override;

    void wake_up() override;

    uint8_t read_rom_value() override;
    uint8_t read_ram_value() override;
    void write_ram_value(uint8_t value) override;
    void set_ram_value(uint8_t col, uint8_t line, uint8_t value) override;

private:
    uint8_t segment_on_value_sp_bs(int curr_line);

    uint8_t read_rom_melody_value();
    void load_new_note(uint8_t* update_melody_adress = nullptr);

    // -- from SM511_2_instruction.cpp ------------------------------ //
    // Opcode -> instruction of processor

    // ROM adress -> All are jump function => not increment program counter
    void op_atpl();
    void op_rtn0();
    void op_rtn1();
    void op_tl();
    void op_tml();
    void op_tm();
    void op_t();

    // RAM adress
    void op_lb();
    void op_sbm();
    void op_incb();
    void op_decb();

    // Data transfert
    void op_bdc();
    void op_exci();
    void op_excd();
    void op_ptw();
    void op_wr();
    void op_ws();
    void op_dta();

    // input - output instructions
    void op_kta();
    void op_atbp();
    void op_atx();
    void op_atl();
    void op_atfc();

    // Arithmetic
    void op_dc();
    void op_rot();

    // Melody control
    void op_pre();
    void op_sme();
    void op_rme();
    void op_tmel();

    // other
    void op_clklo(); // change frequency of cpu -> low = 8.192kHz
    void op_clkhi(); // change frequency of cpu -> medium = 16.384kHz



public : // debug
    uint8_t debug_get_elem_rom(int col, int line, int word) { return rom[col][line][word]; }
    int debug_rom_adress_size_col() override { return SM511_2_ROM_COL; };
    int debug_rom_adress_size_line() override { return SM511_2_ROM_LINE; }

    uint8_t debug_get_elem_ram(int col, int line) override;
    int debug_ram_adress_size_col() override { return SM511_2_RAM_COL; };
    int debug_ram_adress_size_line() override { return SM511_2_RAM_LINE; }

    uint8_t debug_multiplexage() { return w_shift_register; }

};
