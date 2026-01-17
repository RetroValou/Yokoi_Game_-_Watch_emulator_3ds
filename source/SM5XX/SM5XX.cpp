#include "SM5XX/SM5XX.h"
#include "std/timer.h"
#include "virtual_i_o/time_addresses.h"
#include <sys/stat.h>


bool SM5XX::step(){ // loop of CPU
    // output -> is opcode are executed or not
    // execution of 1 cycle of cpu
    bool execution_opcode = false;

    if(cycle_curr_opcode <= 0){
        debug_theorie_time = debug_cycle_curr_opcode * time_per_cycle_us; 
        debug_cycle_previous_opcode = debug_cycle_curr_opcode;
        
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

    return execution_opcode;
}



void SM5XX::execute_cycle(){
    // each execution need during 1 cycle
    step_clock_divider(); // in, there are update screen
    cycle_curr_opcode--;
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

//////////////////////////////////// Time Addresses ////////////////////////////////////

void SM5XX::load_rom_time_addresses(const std::string& ref_game)
{
    time_addresses = get_time_addresses(ref_game);
}

void SM5XX::set_time(uint8_t hour, uint8_t minute, uint8_t second) {
    if (is_time_set()) 
        return; // only set time once

    // Use time_addresses if available, otherwise fallback to default (col 4, lines as before)
    if (time_addresses) {
        // If the hour is greater than 12 then remove 12, and add PM bit
        uint8_t hour_12h_value = hour;
        uint8_t pm_bit = 0x00;
        
        // Skip setting PM bit if pm_bit == 24 (indicates 24-hour clock)
        if (time_addresses->pm_bit != 24) {
            if (hour_12h_value > 12) {
                hour_12h_value -= 12;
                pm_bit = time_addresses->pm_bit;
            }
            else if (hour_12h_value == 12) {
                pm_bit = time_addresses->pm_bit; // Noon case
            }
            else if (hour_12h_value == 0) {
                hour_12h_value = 12; // Midnight case
            }
        }

        // Set the time digits in RAM using per-digit (col, line)
        set_ram_value(time_addresses->col_hour_tens, time_addresses->line_hour_tens, (hour_12h_value / 10) | pm_bit);
        set_ram_value(time_addresses->col_hour_units, time_addresses->line_hour_units, hour_12h_value % 10);
        set_ram_value(time_addresses->col_minute_tens, time_addresses->line_minute_tens, minute / 10);
        set_ram_value(time_addresses->col_minute_units, time_addresses->line_minute_units, minute % 10);
        set_ram_value(time_addresses->col_second_tens, time_addresses->line_second_tens, second / 10);
        set_ram_value(time_addresses->col_second_units, time_addresses->line_second_units, second % 10);
    }
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

///////////////////////////////////// Debug ////////////////////////////////////

void SM5XX::debug_dump_ram_state(const char* filename) {
    // Dump the current RAM state to a file for debugging on the sd card here: "sdmc:/3ds/debug/"
    // check the folder exists
    mkdir("sdmc:/3ds/debug", 0777);

    // Build full path
    std::string full_path = std::string("sdmc:/3ds/debug/") + filename;
    FILE* file = fopen(full_path.c_str(), "a"); // append mode
    if (!file) return;

    // Print header with game name
    fprintf(file, "---- RAM DUMP ----\n");
    fprintf(file, "CPU: %s\n", name_cpu.c_str());

    // Print time (HH:MM:SS)
    time_t rawtime;
    struct tm* timeinfo;
    char time_str[16];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    fprintf(file, "Time: %s \n", time_str);

    int cols = debug_ram_adress_size_col();
    int lines = debug_ram_adress_size_line();

    // Print column header
    fprintf(file, "Cols    :");
    for(int col = 0; col < cols; col++) {
        fprintf(file, " %d", col);
    }
    fprintf(file, "\n");

    // Print separator
    fprintf(file, "-------------------\n");

    // Print RAM contents
    for(int line = 0; line < lines; line++){
        fprintf(file, "Line %02d :", line);
        for(int col = 0; col < cols; col++){
            fprintf(file, " %X", debug_get_elem_ram(col, line) & 0x0F);
        }
        fprintf(file, " \n");
    }
    fprintf(file, "\n");
    fclose(file);
}
