#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>
#include "SM5XX/SM5XX.h"


// protect glitching when screen update during population of W and W_prime register
// -> wait if opcode link to W and W_prime are recently executed
constexpr uint8_t THRESHOLD_CYCLE_UPDATE_W = 40; 

constexpr uint8_t SM5A_RAM_COL = 5;
constexpr uint8_t SM5A_RAM_LINE = 13;

constexpr uint8_t SM5A_ROM_COL = 2;
constexpr uint8_t SM5A_ROM_LINE = 16; // last columns have only 13 lines
// constexpr int ROM_WORD = 63; declared in SM5xx.h

constexpr uint8_t SM5A_ROM_MAME_LINE = 16;
constexpr uint8_t SM5A_ROM_MAME_WORD = 63 + 1; // per each line : 63 standars word + 1 fake word (0x80) = 64 word

constexpr uint8_t SM5A_SEGMENT_COL = 9;
constexpr uint8_t SM5A_SEGMENT_LINE = 4;
constexpr uint8_t SM5A_SEGMENT_WORD = 2;
// 2 word -> w or w' register == multiplex

//  constexpr int FREQUENCY_CPU = 32768; declared in SM5xx.h


class SM5A: public SM5XX
{
public : 
    SM5A() : 
        SM5XX("SM5A\0") // +1 for bs output
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
    uint8_t get_cpu_type_id() override { return 0; } // CPU_TYPE_SM5A

private:
    /// Variables / register /// 
    uint8_t ram[SM5A_RAM_COL][SM5A_RAM_LINE];
    uint8_t rom[SM5A_ROM_COL][SM5A_ROM_LINE][ROM_WORD];

    uint8_t segment_on[SM5A_SEGMENT_COL][SM5A_SEGMENT_LINE];

    // used when program counter col are bigger than ram    
    uint8_t cb_debordement_rom_program_counter;

    // Register used for control segment
    uint8_t w_screen_control[9];
    uint8_t w_prime_screen_control[9];
    uint8_t w_size = 9;
    uint8_t last_w_update; // -> For not update screen when program adding data now on w and not finish

    bool cn_flag;

    bool r_subroutine_flag;
    bool e_temporar_flag;

    uint8_t r_output_control; // used for buzzer (pin R1) and multiplex input
    bool m_flag_segment_decoder;


/// ##### FUNCTION ################################################# ///
private:
    void update_segment() override;

    bool no_pc_increase(uint8_t opcode) override;
    bool is_on_double_octet(uint8_t opcode) override;

    void execute_curr_opcode() override;

    bool condition_to_update_segment() override;

    void wake_up() override;

    uint8_t read_rom_value() override;
    uint8_t read_ram_value() override;
    void write_ram_value(uint8_t value) override;

private:
    // -- from SM5A_instruction.cpp ------------------------------ //
    // Opcode -> instruction of processor

    // ROM adress -> All are jump function => not increment program counter
    void op_comcn();
    void op_rtn(); 
    void op_rtns();
	void op_ssr();
    void op_tr();
	void op_trs();

    // RAM adress
	void op_lb();
    void op_incb();
    void op_decb();
    void op_sbm();
    void op_rbm();

    // Data transfert
    void op_exci();
    void op_excd();
    void op_atbp();
    void op_ptw();
    void op_pdtw();
    void op_tw();
    void op_dtw();
    void op_wr();
    void op_ws();

    // input - output instructions
    void op_atr();
    void op_kta();

    // Divider
    void op_dta();

    // BIT manipulation
    void op_rmf();
    void op_smf();
    void op_comcb();

    // other
    void op_skip(); 




public : // debug
    int debug_rom_adress_size_col() override { return SM5A_ROM_COL; };
    int debug_rom_adress_size_line() override { return SM5A_ROM_LINE; }

    uint8_t debug_get_elem_ram(int col, int line) override;
    uint8_t debug_get_elem_rom(int col, int line, int word) override { return rom[col][line][word]; }
    int debug_ram_adress_size_col() override { return SM5A_RAM_COL; };
    int debug_ram_adress_size_line() override { return SM5A_RAM_LINE; }


    uint8_t debug_multiplexage() override { return r_output_control; }
    uint8_t debug_cb_debordement() override { return cb_debordement_rom_program_counter; }

    uint8_t debug_w_screen(int i) override {return w_screen_control[i];};
    uint8_t debug_w_prime_screen(int i) override {return w_prime_screen_control[i];};

    uint8_t debug_CN_Flag() override { return cn_flag; }

};
