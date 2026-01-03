#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>
#include "SM5XX/SM5XX.h"


constexpr uint8_t SM510_RAM_COL = 8;
constexpr uint8_t SM510_RAM_LINE = 16;
constexpr uint8_t SM510_RAM_VIDEO_COL = 2;

constexpr uint8_t SM510_ROM_COL = 4;
constexpr uint8_t SM510_ROM_LINE = 11;
// constexpr int ROM_WORD = 63; declared in SM5xx.h

constexpr uint8_t SM510_ROM_MAME_LINE = 16; // per each col : 11 standars lines + 5 fake lines (0x00...) = 16 lines
constexpr uint8_t SM510_ROM_MAME_WORD = 63 + 1; // per each line : 63 standars word + 1 fake word (0x80) = 64 word

//  constexpr int FREQUENCY_CPU = 32768; declared in SM5xx.h


class SM510: public SM5XX {
public : 
    SM510() : 
        SM5XX("SM510\0") // +1 for bs output
        {}

public:
    void init() override;
    void load_rom(const uint8_t* file_hex, size_t size_hex) override;

    bool screen_is_on() override;
    bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) override;
    bool get_active_sound() override;

    // Save/Load state
    bool save_state(FILE* file) override;
    bool load_state(FILE* file) override;
    uint8_t get_cpu_type_id() override { return 1; } // CPU_TYPE_SM510

private:
    /// Variables / register /// 
    uint8_t ram[SM510_RAM_COL][SM510_RAM_LINE];
    uint8_t rom[SM510_ROM_COL][SM510_ROM_LINE][ROM_WORD];

    uint8_t segment_on[SM510_RAM_VIDEO_COL+1][SM510_RAM_LINE];

    ProgramCounter r_buffer_program_counter; // second buffer of program counter

    // input
    uint8_t w_shift_register; // Used for input -> Sequential
    
    uint8_t r_buzzer_control;
    uint8_t r_buzzer_output;

    bool bc_lcd_stop;

    uint8_t l_bs; // connected to bs -> blink output
    uint8_t y_bs; 

    uint8_t alternativ_col_ram;

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

    // -- from SM510_instruction.cpp ------------------------------ //
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
    void op_wr();
    void op_ws();

    // input - output instructions
    void op_kta();
    void op_atbp();
    void op_atl();
    void op_atfc();
    void op_atr();

    // Arithmetic
    void op_rot();

    // Test -> if, ...
    void op_tf1();
    void op_tf4();

    // other
    void op_skip(); // no instruction, program counter +1


public : // debug
    uint8_t debug_get_elem_rom(int col, int line, int word) override { return rom[col][line][word]; }
    int debug_rom_adress_size_col() override { return SM510_ROM_COL; };
    int debug_rom_adress_size_line() override { return SM510_ROM_LINE; }

    uint8_t debug_get_elem_ram(int col, int line) override;
    int debug_ram_adress_size_col() override { return SM510_RAM_COL; };
    int debug_ram_adress_size_line() override { return SM510_RAM_LINE; }

    uint8_t debug_multiplexage() override { return w_shift_register; }
};
