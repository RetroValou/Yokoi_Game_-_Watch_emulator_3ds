#pragma once
#include <vector>
#include <stdint.h>
#include <string>
#include <SM5XX\Base_Structure.h>
#include "virtual_i_o/time_addresses.h"


constexpr uint8_t ROM_WORD = 63; // each SM5XX have 63 rom word -> it's why program counter is the same
constexpr uint32_t FREQUENCY_CPU = 32768; // Hz = 32,768 kHZ => 30.517us 1 cycle => 61us for almost instruction



class SM5XX {
public : 
    SM5XX(const std::string& name) :
        name_cpu(name),
        frequency(FREQUENCY_CPU),
        time_per_cycle_us(1'000'000.0 / FREQUENCY_CPU)
        {}

public:
    std::string name_cpu;
    bool stop_cpu = false;
    bool segments_state_are_update = false;
    bool input_no_multiplex = false;
    uint32_t frequency;
    uint32_t sound_divide_frequency = 1; 

protected:
    // cpu logic
    uint8_t curr_opcode;
    int cycle_curr_opcode; // cycles used (in theorie) for perform current opcode

    // move in ram and rom
    ProgramCounter program_counter; // current Rom Adress 
    ProgramCounter s_buffer_program_counter; // buffer of rom adress
    RAMAddress ram_address;    

    // used for calculation / variable -> like register
    bool carry; 
    uint8_t accumulator;
    
    // mesure time
    uint16_t f_clock_divider;
    bool gamma_flag_second; // if 1sec is past -> True. set False by opcode

    // input
    uint8_t k_input[8]; // 8 enter K (4bit) (only 4 on SM5A) by default = 0
    bool beta_input; // special input, by default = 1
    bool alpha_input; // special input, by default = 1

    // screen
    bool bp_lcd_blackplate; // On the backplate part of screen

    // execution of cpu
    uint8_t cpu_frequency_divider = 0x01;
    double time_per_cycle_us;

    // other variables
    bool is_sleep; // cpu sleep -> low consumption (true mode on SM5xx)
    uint8_t flag_time_update_screen; // used for update screen in good time

    bool time_set_state = false;

    // time addresses
    const TimeAddress *time_addresses;

public:
    bool step();
    void execute_cycle();
    void input_set(int group, int line, bool state);

    void load_rom_time_addresses(const std::string& ref_game);
    void time_set(bool state){ time_set_state = state; }
    bool is_time_set(){ return time_set_state; }
    void set_time(uint8_t hour, uint8_t minute, uint8_t second);

private : 
    void adding_program_counter(const uint8_t* opcode);
    void calculate_cycle(uint8_t opcode);
    void step_clock_divider();

    void wait_timing_cpu(int cycle);
    uint64_t get_target_time_cpu(int cycle);

    bool condition_to_wake_up();
    bool check_button_pressed();

protected : 
    uint8_t get_parameter_of_opcode(bool add_pc = true);
    void skip_instruction();
    void copy_buffer(const ProgramCounter& src, ProgramCounter& dst); // usefull for some SM5XX CPU






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Differences between each CPU ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Each cpu need to have this function but declaration of functions are differents due to structure of each CPU

public : 
    virtual void init() = 0;
    virtual void load_rom(const uint8_t* file_hex, size_t size_hex) = 0;
    virtual void load_rom_melody(const uint8_t*, size_t) { }; // empty for SM5A and SM510. Used only by SM511/2

    virtual bool get_segments_state(uint8_t col, uint8_t line, uint8_t word) = 0;
    virtual bool screen_is_on() = 0;

    virtual bool get_active_sound(){ return false; };

    // Save/Load state for save states
    virtual bool save_state(FILE* file) = 0;
    virtual bool load_state(FILE* file) = 0;
    virtual uint8_t get_cpu_type_id() = 0; // Return CPU type identifier

private :
    virtual void execute_curr_opcode() = 0; // switch case op_code function with curr hexa op_code value

    virtual void update_sound(){}; // SM511/2 need update sound logic (melody). Other no need (in theorie need but simplification work great)

    virtual bool no_pc_increase(uint8_t opcode) = 0; // need for adding_program_counter -> say opcode with not increase PC
    virtual bool is_on_double_octet(uint8_t opcode) = 0; // need for instruction on 2 octet -> skip 2 octet
    
    virtual void update_segment() = 0; // maj of segment -> depend of structure of """RAM video""" of each CPU (SM5A are w register, SM510 and SM511/2 RAM)
    virtual bool condition_to_update_segment() = 0;

    virtual void wake_up() = 0; // initisalisation of cpu after a wake up

protected:
    virtual uint8_t read_rom_value() = 0;
    virtual uint8_t read_ram_value() = 0;
    virtual void write_ram_value(uint8_t value) = 0;
    virtual void set_ram_value(uint8_t col, uint8_t line, uint8_t value) = 0;





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Opcode -> instruction of processor //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// -- from SM5XX_instruction.cpp ------------------------------ //
// All here are the same between SM5A, SM510, SM511 and SM512

protected: 

    // RAM adress
    void g_op_lbl(); // move ram
    void g_op_exbla(); // ram_line <-> acc

    // Data transfert
    void g_op_lax(); // acc = op_code
    void g_op_lda(); // acc = ram / move ram columns
    
    // Arithmetic
    void g_op_add(); // acc = acc+ram
    void g_op_add11(); // acc = acc+ram+carry / set carry
    void g_op_adx(); // acc = acc+op_code
    void g_op_coma(); // acc = ~acc
    void g_op_rc(); // carry = 0
    void g_op_sc(); // carry = 1

    // Data transfert
    void g_op_exc(); // acc <-> ram / move ram columns
    
    // Test -> if, ...
    void g_op_ta(); // alpha_Pin == 1 -> skip
    void g_op_tb(); // beta_Pin == 1 -> skip
    void g_op_tc(); // carry == 0 -> skip
    void g_op_tam(); // acc == ram -> skip 
    void g_op_tis(); // 1sec is past (gamma) -> skip 
    void g_op_tmi(); // bit x of ram == 1 -> skip
    void g_op_ta0(); // acc == 0x00 -> skip 
    void g_op_tabl(); // acc == ram_line -> skip 

    // BIT manipulation
    void g_op_rm(); // change ram bit x per 0
    void g_op_sm(); // change ram bit x per 1

    // other
    void g_op_cend(); // stop clock
    void g_op_idiv(); // reset clock divider
    void g_op_illegal(); // op code not exit -> Stop CPU





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Debug ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public :
    bool debug_gamma_flag(){ return gamma_flag_second; }
    uint16_t debug_divider_time(){ return f_clock_divider; }

    int64_t debug_time_wait;
    int64_t debug_time_need;
    int64_t debug_opcode_time;
    uint32_t debug_nb_jump_LAX;
    uint64_t debug_theorie_time;
    uint8_t debug_cycle_previous_opcode;
    uint8_t debug_cycle_curr_opcode;
    uint16_t debug_curr_opcode;

    int debug_program_counter_col() { return program_counter.col; }
    int debug_program_counter_line() { return program_counter.line; }
    int debug_program_counter_word() { return program_counter.word; }
    uint8_t debug_k_input(int i){ return k_input[i]; }
    uint8_t debug_accumulator(){ return accumulator; }
    int debug_s_buffer_program_counter_col() { return s_buffer_program_counter.col; }
    int debug_s_buffer_program_counter_line() { return s_buffer_program_counter.line; }
    int debug_s_buffer_program_counter_word() { return s_buffer_program_counter.word; }
    int debug_ram_adress_col(){ return ram_address.col; }
    int debug_ram_adress_line(){ return ram_address.line; }

    void debug_dump_ram_state(const char* filename);

    virtual uint8_t debug_get_elem_rom(int, int, int) { return 0x00; }
    virtual int debug_rom_adress_size_col(){ return 0; }
    virtual int debug_rom_adress_size_line(){ return 0; }
    virtual uint8_t debug_get_elem_ram(int, int) { return 0x00; }
    virtual int debug_ram_adress_size_col(){ return 0; }
    virtual int debug_ram_adress_size_line(){ return 0; }

    virtual uint8_t debug_multiplexage() {return 0x00; }
    virtual uint8_t debug_cb_debordement(){return 0x00; }
    virtual uint8_t debug_w_screen(int){ return 0x00; }
    virtual uint8_t debug_w_prime_screen(int){ return 0x00; }
    virtual uint8_t debug_CN_Flag(){ return 0x00; }

};
