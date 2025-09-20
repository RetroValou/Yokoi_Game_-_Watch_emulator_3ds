#include "SM5XX/SM511_SM512/sm511_2.h"

void SM511_2::execute_curr_opcode() {
	switch (curr_opcode & 0xf0)
	{
		case 0x20: g_op_lax(); break;
		case 0x30: g_op_adx(); break;
		case 0x40: op_lb(); break;
		case 0x70: op_tl(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0: // 10xx xxxx
			op_t(); break;
		case 0xc0: case 0xd0: case 0xe0: case 0xf0: // 11xx xxxx
			op_tm(); break;

		default:
			switch (curr_opcode & 0xfc)
			{
                case 0x04: g_op_rm(); break;
                case 0x0c: g_op_sm(); break;
                case 0x10: g_op_exc(); break;
                case 0x14: op_exci(); break;
                case 0x18: g_op_lda(); break;
                case 0x1c: op_excd(); break;
                case 0x54:  g_op_tmi(); break;
                case 0x68: op_tml(); break;

                default:
                    switch (curr_opcode)
                    {
                        case 0x00: op_rot(); break;
                        case 0x01: op_dta(); break;
                        case 0x02: op_sbm(); break;
                        case 0x03: op_atpl(); break;
                        case 0x08: g_op_add(); break;
                        case 0x09: g_op_add11(); break;
                        case 0x0a: g_op_coma(); break;
                        case 0x0b: g_op_exbla(); break;

                        case 0x50: op_kta(); break;
                        case 0x51: g_op_tb(); break;
                        case 0x52: g_op_tc(); break;
                        case 0x53: g_op_tam(); break;
                        case 0x58: g_op_tis(); break;
                        case 0x59: op_atl(); break;
                        case 0x5a: g_op_ta0(); break;
                        case 0x5b: g_op_tabl(); break;
                        case 0x5c: op_atx(); break;
                        case 0x5d: g_op_cend(); break;
                        case 0x5e: g_op_ta(); break;
                        case 0x5f: g_op_lbl(); break;

                        case 0x61: op_pre(); break;
                        case 0x62: op_wr(); break;
                        case 0x63: op_ws(); break;
                        case 0x64: op_incb(); break;
                        case 0x65: g_op_idiv(); break;
                        case 0x66: g_op_rc(); break;
                        case 0x67: g_op_sc(); break;
                        case 0x6c: op_decb(); break;
                        case 0x6d: op_ptw(); break;
                        case 0x6e: op_rtn0(); break;
                        case 0x6f: op_rtn1(); break;

                        // extended opcodes
                        case 0x60:
                            curr_opcode = get_parameter_of_opcode();
                            debug_curr_opcode = (debug_curr_opcode<<8) | curr_opcode;
                            switch (curr_opcode)
                            {
                                case 0x30: op_rme(); break;
                                case 0x31: op_sme(); break;
                                case 0x32: op_tmel(); break;
                                case 0x33: op_atfc(); break;
                                case 0x34: op_bdc(); break;
                                case 0x35: op_atbp(); break;
                                case 0x36: op_clkhi(); break;
                                case 0x37: op_clklo(); break;

                                default: g_op_illegal(); break;
                            }
                            break; // 0x60
                        default: g_op_illegal(); break;
                    }
                    break; // 0xff
            }
            break; // 0xfc
	} // 0xf0

    // 0x02 = SBM instruction use for temporaly change col ram (only for next instruction)
    if( (alternativ_col_ram != 0x00) && (curr_opcode != 0x02) ){ 
        alternativ_col_ram = 0x00;
    }
}


// ############### Opcode -> instruction of processor #############################################

// ----- ROM adress 
// -> All are jump function => not increment program counter
void SM511_2::op_atpl(){ // very low jump ""calculate""
    program_counter.word = (program_counter.word & 0x30) | (accumulator & 0x0F);
};

void SM511_2::op_rtn0(){ // pop all from buffer 
    copy_buffer(s_buffer_program_counter, program_counter);
    copy_buffer(r_buffer_program_counter, s_buffer_program_counter);
};

void SM511_2::op_rtn1(){ // same to rtn0 with skip instruction
    op_rtn0();
    skip_instruction();
};

void SM511_2::op_tl(){ // long jump -> all ROM
    uint8_t param = get_parameter_of_opcode(false);
    program_counter.col = (param & 0xC0) >> 6;    
    program_counter.line = curr_opcode & 0x0F;
    program_counter.word = param & 0x3F;
};

void SM511_2::op_tml(){ // long jump with buffering
    uint8_t param = get_parameter_of_opcode();
    copy_buffer(s_buffer_program_counter, r_buffer_program_counter);
    copy_buffer(program_counter, s_buffer_program_counter);

    program_counter.col = (param & 0xC0) >> 6;    
    program_counter.line = curr_opcode & 0x03;
    program_counter.word = param & 0x3F;
};

void SM511_2::op_tm(){ // go to """subroutine"""
    // push all on buffer -> move all
    copy_buffer(s_buffer_program_counter, r_buffer_program_counter);
    copy_buffer(program_counter, s_buffer_program_counter);

    // Like subroutine
    // Jump to begin of ROM -> multiple 'function' are declare here (adress of function)
    program_counter.col = 0x00; 
    program_counter.line = 0x00; 
    program_counter.word = curr_opcode & 0x3F;
    uint8_t param = get_parameter_of_opcode(false);
    cycle_curr_opcode += 2; // because read rom

    program_counter.col = (param & 0xC0) >> 6;    
    program_counter.line = 0x04; 
    program_counter.word = param & 0x3F;
};

void SM511_2::op_t(){ // short jump -> word to word
    program_counter.word = curr_opcode & 0x3F;
};

// ----- RAM adress
void SM511_2::op_lb(){ // set address RAM in the most bizarre way possible (probably optimization of ROM space)
    ram_address.col = (ram_address.col & 0x0C) | (curr_opcode & 0x03);
    
    ram_address.line = ((curr_opcode & 0x0C) >> 2);
    if( (curr_opcode & 0x0C) != 0x00){
        ram_address.line = 0x0C | ram_address.line;
    }
};

void SM511_2::op_sbm(){
    alternativ_col_ram = ram_address.col | 0x04;
};

void SM511_2::op_incb(){
    if(ram_address.line == SM511_2_RAM_LINE-1)
    { 
        skip_instruction();
        ram_address.line = 0;
    }
    else { ram_address.line = ram_address.line + 1; }
};

void SM511_2::op_decb(){
    if(ram_address.line == 0)
    { 
        skip_instruction();
        ram_address.line = SM511_2_RAM_LINE-1;
    } 
    else { ram_address.line = ram_address.line - 1; }
};

// ----- Data transfert
void SM511_2::op_bdc(){ bc_lcd_stop = carry; };

void SM511_2::op_exci(){
    g_op_exc();
    op_incb();
};

void SM511_2::op_excd()
{
    g_op_exc();
    op_decb();
};


void SM511_2::op_ptw(){ s_pin = w_shift_register; }; // push register w to s pin for multiplexage input

void SM511_2::op_wr(){ w_shift_register = w_shift_register << 1; };
void SM511_2::op_ws(){ w_shift_register =(w_shift_register << 1) | 0x01; };

void SM511_2::op_dta(){ // not find on sharp doc but on mame and rom (Super Mario Bros). Why ???
    accumulator = (f_clock_divider >> 11) & 0x0f;
}

// ----- input - output instructions
void SM511_2::op_kta(){
    accumulator = 0x00;
    for(int i = 0; i < 8; i++){
        if( ((s_pin >> i) & 0x01) == 0x01){ // multiplexage by S
            accumulator = accumulator | k_input[i];
        }
    }
};

void SM511_2::op_atbp(){ bp_lcd_blackplate = accumulator & 0x01; };
void SM511_2::op_atl(){ l_bs = accumulator; };
void SM511_2::op_atfc(){ y_bs = accumulator; };
void SM511_2::op_atx(){ x_bs = accumulator; };

// ----- Arithmetic
void SM511_2::op_rot(){
    bool tmp = accumulator & 0x01;
    accumulator = (accumulator >> 1) | (carry << 3);
    carry = tmp;
};


// ----- Melody Transfert
void SM511_2::op_pre(){
    uint8_t param = get_parameter_of_opcode();
    load_new_note(&param);
}

void SM511_2::op_sme(){
    me_melody_activate = true;
}

void SM511_2::op_rme(){
    me_melody_activate = false;
}

void SM511_2::op_tmel(){
    if(mes_melody_finish){ 
        skip_instruction(); 
        mes_melody_finish = false;
    }
}

void SM511_2::op_clklo(){ // low frequency -> low comsumption (and low speed !)
    cpu_frequency_divider = 2;
} 
void SM511_2::op_clkhi(){// normal frequency
    cpu_frequency_divider = 1;
} 



