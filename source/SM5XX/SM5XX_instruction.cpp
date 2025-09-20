#include "SM5XX/SM5XX.h"


// Instruction of cpu we can find on each SM5XX (SM5A / SM510 / SM511 / SM512)
// with no difference on execution


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// RAM adress //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM5XX::g_op_lbl(){
    uint8_t param = get_parameter_of_opcode();
    ram_address.col = (param & 0x70) >> 4;
    ram_address.line = param & 0x0F;
};

void SM5XX::g_op_exbla(){
    uint8_t tmp = ram_address.line;
    ram_address.line = accumulator;
    accumulator = tmp;
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Data transfert //////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM5XX::g_op_lax(){
    accumulator = 0x0F & curr_opcode;
    debug_nb_jump_LAX = 0;
    while( (0xF0 & read_rom_value()) == 0x20){ // jump all lax instruction after -> only one executed
        skip_instruction();
        debug_nb_jump_LAX += 1;
    }
};

void SM5XX::g_op_lda(){
    accumulator = read_ram_value();
    ram_address.col = (ram_address.col ^ (curr_opcode & 0x03)) & 0x07;
};

void SM5XX::g_op_exc(){
    // swap ram and accumulator
    uint8_t tmp = read_ram_value();
    write_ram_value(accumulator);
    accumulator = tmp;
    ram_address.col = ram_address.col ^ (curr_opcode & 0x03);
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Arithmetic //////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM5XX::g_op_add(){
    accumulator = (accumulator + read_ram_value()) & 0x0F;
};

void SM5XX::g_op_add11(){
    accumulator = accumulator + read_ram_value() + carry;
    carry = (accumulator > 0x0F);
    if(carry) { skip_instruction(); }
    accumulator = accumulator & 0x0F;
};

void SM5XX::g_op_adx(){
    accumulator = accumulator + (curr_opcode & 0x0F);
    if( (accumulator > 0x0F) && ( (curr_opcode & 0x0F) != 0x0A) ) // bug of cpu with value 0x0A
        { skip_instruction(); }
    accumulator = accumulator & 0x0F;
};

void SM5XX::g_op_coma(){
    accumulator = accumulator ^ 0x0F;
};

void SM5XX::g_op_rc(){ carry = 0; };
void SM5XX::g_op_sc(){ carry = 1; };



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Test -> if, ... skip ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM5XX::g_op_ta(){
    if(alpha_input) { skip_instruction(); }
};

void SM5XX::g_op_tb(){
    if(beta_input) { skip_instruction(); }
};

void SM5XX::g_op_tc(){
    if(!carry) { skip_instruction(); }
};

void SM5XX::g_op_tam(){
    if(accumulator == read_ram_value()) { skip_instruction(); }
};

void SM5XX::g_op_tis(){
    if(!gamma_flag_second){ skip_instruction(); }
    gamma_flag_second = false;
};

void SM5XX::g_op_tmi(){
    // check if bit x of RAM is equal to 1
    if( ((read_ram_value() >> (curr_opcode & 0x03)) & 0x01) == 1){ 
        skip_instruction();
    }
};

void SM5XX::g_op_ta0(){
    if(accumulator == 0x00){ skip_instruction(); }
};

void SM5XX::g_op_tabl(){
    if(accumulator == ram_address.line){ skip_instruction(); }
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// BIT manipulation ///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM5XX::g_op_rm(){ // change 1byte per 0 in ram
    uint8_t tmp = (~(0x01 << (curr_opcode & 0x03)))&0x0F;
    tmp = read_ram_value() & tmp;
    write_ram_value(tmp);
};

void SM5XX::g_op_sm(){ // change 1byte per 1 in ram
    uint8_t tmp = 0x01 << (curr_opcode & 0x03);
    tmp = read_ram_value() | tmp;
    write_ram_value(tmp);
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Other ///////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM5XX::g_op_cend(){ // stop clock = cpu is freeze (low power) and reactivate by input or each 1s (very useful for watch !)
    is_sleep = true;
}; 

void SM5XX::g_op_idiv(){ // reset clock divider
    f_clock_divider = 0x0000;
}; 

void SM5XX::g_op_illegal(){ // opcode not exist
    stop_cpu = true;
}; 


