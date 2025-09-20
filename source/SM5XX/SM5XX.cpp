#include "SM5XX/SM5XX.h"
#include "std/timer.h"


bool SM5XX::step(){ // loop of CPU
    // output -> is opcode are executed or not
    // execution of 1 cycle of cpu
    bool execution_opcode = false;

    if(cycle_curr_opcode <= 0){
        debug_theorie_time = debug_cycle_curr_opcode * time_per_cycle_us; 
        debug_cycle_previous_opcode = debug_cycle_curr_opcode;
        debug_time_execute = ((int64_t)time_us_64_p() - debug_time_execute);
        
        execution_opcode = true;
        cycle_curr_opcode = 0x00; // safety -> never less than 0

        if(is_sleep && condition_to_wake_up()){ wake_up(); }

        if(!is_sleep){
            curr_opcode = read_rom_value(); debug_curr_opcode = curr_opcode;
            calculate_cycle(curr_opcode);
            adding_program_counter(&curr_opcode);
            execute_curr_opcode();
            cycle_curr_opcode = cycle_curr_opcode * cpu_frequency_divider; // for increase time of execution 
                        // -> low consumption divide frequency by 2 => 2x more cycles to execute opcode
        }
        else { // for run step_clock is sleep
            cycle_curr_opcode = 1; 
        } 
        debug_cycle_curr_opcode = cycle_curr_opcode;
    }
    execute_cycle();
    if(cycle_curr_opcode <= 0){ debug_time_execute = (int64_t)time_us_64_p(); }

    return execution_opcode;
}



void SM5XX::execute_cycle(){
    // each execution need during 1 cycle
    step_clock_divider(); // in, there are update screen
    cycle_curr_opcode--;
    wait_timing_cpu(1);
    update_sound();
}



void SM5XX::adding_program_counter(const uint8_t* opcode){
    if(opcode == nullptr || !no_pc_increase(*opcode)){
        // swift bit word counter to right + adding new bit (w[0] == w[1]) (1 if equal, 0 if not)
        bool tmp = (program_counter.word & 0x01) == ((program_counter.word & 0x02) >> 1);
        program_counter.word = program_counter.word >> 1;
        program_counter.word = (tmp << 5) | program_counter.word;
    }
}



void SM5XX::calculate_cycle(uint8_t opcode){
    // Calculate nb cycle cpu need to execute a opcode 
    // if opcode on 1 octet = 2 cycles (1 octet = 2*4bit. cpu is 4bit)
    // if opcode on 2 octet = 4 cycles 
    cycle_curr_opcode += 2;
    if(is_on_double_octet(opcode)){ cycle_curr_opcode += 2; }
}



void SM5XX::skip_instruction(){
    // skip a instruction. This fonction is call on opcode function
    // need to wait same nb cycle of instruction skiped
    uint8_t opcode_to_skip = read_rom_value(); // PC already set on new opcode we skip
    calculate_cycle(opcode_to_skip);
    adding_program_counter(nullptr); // NO NEED OPCODE -> empty opcode
    if(is_on_double_octet(opcode_to_skip)){ adding_program_counter(nullptr); }
}




/////////////////////////////////// Timing CPU ///////////////////////////////////

uint64_t SM5XX::get_target_time_cpu(int cycle){
    uint64_t target_time;
    nb_group_cycle += cycle;
    target_time = time_last_group_cycle 
                    + (uint64_t)(nb_group_cycle * time_per_cycle_us + 0.5);
    
    if(nb_group_cycle > 200){ // protection contre derive precision
        nb_group_cycle = 0;
        time_last_group_cycle = target_time;
    }
    return target_time;
}


void SM5XX::wait_timing_cpu(int cycle){
    if(cycle <= 0){ return; }

    uint64_t target_time = get_target_time_cpu(cycle);
    int64_t time_wait = ((int64_t)target_time) - ((int64_t)time_us_64_p());
    if(!NO_WAIT_CYCLE && time_wait > 0){ sleep_us_p(time_wait); }
}



/////////////////////////////// Clock divider cpu -> segment calibrate with ///////////////////////////////

void SM5XX::step_clock_divider(){
    // each cycle, signal are divide (15 times) for obtain 1 seconds
    f_clock_divider = (f_clock_divider + 1) & 0x7FFF; // on 15 bit
    if(f_clock_divider == 0x00) {
        gamma_flag_second = true; // reset by instruction, not automaticly
    }
    if(condition_to_update_segment()) { update_segment(); } // depending to clock value
}



//////////////////////////////////// Input ////////////////////////////////////
void SM5XX::input_set(int group, int line, bool state){
    // special input -> says by line >= 8 (not exist in true K input)
    if(line == 8){ alpha_input = state; }
    else if(line == 9){ beta_input = state; }

    else if(state){ k_input[group] = k_input[group] | (0x01 << line); }
    else { k_input[group] = k_input[group] & ~(0x01 << line); }
}



//////////////////////////////////// Wake up ////////////////////////////////////

bool SM5XX::check_button_pressed(){ // used for detect wake up
    uint8_t result = 0x00;
    for(int i = 0; i < 8; i++){ result |= k_input[i]; }
    if(!alpha_input || !beta_input){ result |= 0x01; }
    return (result != 0x00);
}

bool SM5XX::condition_to_wake_up(){ // wake up when bouton are pressed or 1sec is past
    return (gamma_flag_second | check_button_pressed()); 
}



//////////////////////////////////// Usefull function ////////////////////////////////////

uint8_t SM5XX::get_parameter_of_opcode(bool add_pc){
    uint8_t param = read_rom_value();
    if(add_pc){ adding_program_counter(nullptr); } // move to next instruction because now we are on parameter value
    return param;
}


void SM5XX::copy_buffer(const ProgramCounter& src, ProgramCounter& dst) {
    dst.col = src.col;
    dst.line = src.line;
    dst.word = src.word;
}


