#include "SM5XX/SM5A/sm5A.h"

static const uint8_t lut_digits[0x20] = // default digit segments PLA
{
	0xe, 0x0, 0xc, 0x8, 0x2, 0xa, 0xe, 0x2, 0xe, 0xa, 0x0, 0x0, 0x2, 0xa, 0x2, 0x2,
	0xb, 0x9, 0x7, 0xf, 0xd, 0xe, 0xe, 0xb, 0xf, 0xf, 0x4, 0x0, 0xd, 0xe, 0x4, 0x0
};

void SM5A::execute_curr_opcode() {
    switch (curr_opcode & 0xf0)
	{
		case 0x20: g_op_lax(); break;
		case 0x30: g_op_adx(); break;
		case 0x40: op_lb(); break;
		case 0x70: op_ssr(); break;

		case 0x80: case 0x90: case 0xa0: case 0xb0:
			op_tr(); break;
		case 0xc0: case 0xd0: case 0xe0: case 0xf0:
			op_trs(); break;

		default:
			switch (curr_opcode & 0xfc)
			{
				case 0x04: g_op_rm(); break;
				case 0x0c: g_op_sm(); break;
				case 0x10: g_op_exc(); break;
				case 0x14: op_exci(); break;
				case 0x18: g_op_lda(); break;
				case 0x1c: op_excd(); break;
				case 0x54: g_op_tmi(); break;

				default:
					switch (curr_opcode)
					{
						case 0x00: op_skip(); break;
						case 0x01: op_atr(); break;
						case 0x02: op_sbm(); break;
						case 0x03: op_atbp(); break;
						case 0x08: g_op_add(); break;
						case 0x09: g_op_add11(); break;
						case 0x0a: g_op_coma(); break;
						case 0x0b: g_op_exbla(); break;

						case 0x50: g_op_ta(); break;
						case 0x51: g_op_tb(); break;
						case 0x52: g_op_tc(); break;
						case 0x53: g_op_tam(); break;
						case 0x58: g_op_tis(); break;
						case 0x59: op_ptw(); break;
						case 0x5a: g_op_ta0(); break;
						case 0x5b: g_op_tabl(); break;
						case 0x5c: op_tw(); break;
						case 0x5d: op_dtw(); break;
						case 0x5f: g_op_lbl(); break;

						case 0x60: op_comcn(); break;
						case 0x61: op_pdtw(); break;
						case 0x62: op_wr(); break;
						case 0x63: op_ws(); break;
						case 0x64: op_incb(); break;
						case 0x65: g_op_idiv(); break;
						case 0x66: g_op_rc(); break;
						case 0x67: g_op_sc(); break;
						case 0x68: op_rmf(); break;
						case 0x69: op_smf(); break;
						case 0x6a: op_kta(); break;
						case 0x6b: op_rbm(); break;
						case 0x6c: op_decb(); break;
						case 0x6d: op_comcb(); break;
						case 0x6e: op_rtn(); break;
						case 0x6f: op_rtns(); break;

						// extended opcodes
						case 0x5e:
                            curr_opcode = get_parameter_of_opcode();
                            debug_curr_opcode = (debug_curr_opcode<<8) | curr_opcode;
                            switch (curr_opcode)
							{
								case 0x00: g_op_cend(); break;
								case 0x04: op_dta(); break;

								default: g_op_illegal(); break;
							}
							break; // 0x5e
							default: g_op_illegal(); break;
						}
					break; // 0xff
			}
			break; // 0xfc
	} // 0xf0

	// 0x70-0x7F = SSRx instruction use for temporaly change e_flag (only for next instruction)
    if( e_temporar_flag && ( (curr_opcode & 0xF0) != 0x70) ){ 
        e_temporar_flag = false;
    }

}



// ROM adress -> All are jump function => not increment program counter

void SM5A::op_rtn(){ 
	copy_buffer(s_buffer_program_counter, program_counter);
	r_subroutine_flag = 0;
 }; 

void SM5A::op_rtns(){ 
	op_rtn();
	skip_instruction();
};

void SM5A::op_ssr(){ 
	e_temporar_flag = true;
	s_buffer_program_counter.line = curr_opcode & 0x0f;
};

void SM5A::op_tr(){ 	
	if(r_subroutine_flag == 0){
		program_counter.word = curr_opcode & 0x3F;
		program_counter.line = s_buffer_program_counter.line;
		program_counter.col = cb_debordement_rom_program_counter;
	}
	else {
		program_counter.word = curr_opcode & 0x3F;
	}
 };

void SM5A::op_trs(){ 
	if(!r_subroutine_flag){
		r_subroutine_flag = true;
		if(!e_temporar_flag){
			copy_buffer(program_counter, s_buffer_program_counter);
			program_counter.col = 1;
			program_counter.line = 0;
			program_counter.word = curr_opcode & 0x3F;
		}
		else {
			uint8_t tmp_line = s_buffer_program_counter.line;
			copy_buffer(program_counter, s_buffer_program_counter);
			program_counter.col = cb_debordement_rom_program_counter;
			program_counter.line = tmp_line;
			program_counter.word = curr_opcode & 0x3F;
		}
	}
	else {
		program_counter.line = (program_counter.line & 0x0C) | ((curr_opcode & 0x30) >> 4);
		program_counter.word = (curr_opcode & 0x0F);
	}
};

// RAM adress
void SM5A::op_lb(){ // set address RAM in the most bizarre way possible (probably optimization of ROM space)
    ram_address.col = curr_opcode & 0x03;
    
    ram_address.line = ((curr_opcode & 0x0C) >> 2);
    if( (curr_opcode & 0x0C) != 0x00){
        ram_address.line = 0x08 | ram_address.line;
    }
};

void SM5A::op_incb(){
    if(ram_address.line == 16-1) // not overflow
    	{ ram_address.line = 0; }
    else { ram_address.line = ram_address.line + 1; }
	if(ram_address.line == 8) { skip_instruction(); } // strange... error of mame ???
};

void SM5A::op_decb(){
    if(ram_address.line == 0)
    { 
        skip_instruction();
        ram_address.line = 16-1;
    } 
    else { ram_address.line = ram_address.line - 1; }
};

void SM5A::op_sbm(){
    ram_address.col = ram_address.col | 0x04;
};
void SM5A::op_rbm(){ 
	ram_address.col = ram_address.col & (~0x04);
};

// Data transfert

void SM5A::op_exci(){
    g_op_exc();
    op_incb();
};

void SM5A::op_excd()
{
    g_op_exc();
    op_decb();
};

void SM5A::op_atbp(){ 
	bp_lcd_blackplate = accumulator&0x01;
	cn_flag = ((accumulator&0x08) >> 3);
};

void SM5A::op_ptw(){ 
	w_screen_control[w_size-2] = w_prime_screen_control[w_size-2];
	w_screen_control[w_size-1] = w_prime_screen_control[w_size-1];
	last_w_update = THRESHOLD_CYCLE_UPDATE_W;
};

void SM5A::op_pdtw(){ 
	w_prime_screen_control[w_size-2] = w_prime_screen_control[w_size-1];
	w_prime_screen_control[w_size-1] = lut_digits[(cn_flag << 4) |accumulator];
	w_prime_screen_control[w_size-1] |= static_cast<uint8_t>((!cn_flag) && m_flag_segment_decoder);
	last_w_update = THRESHOLD_CYCLE_UPDATE_W;
	cycle_curr_opcode += 2;
};

void SM5A::op_tw(){ 
	for (int i = 0; i < w_size; i++) { w_screen_control[i] = w_prime_screen_control[i]; };
	last_w_update = THRESHOLD_CYCLE_UPDATE_W;
};

void SM5A::op_dtw(){ 
	for (int i = 0; i < w_size-1; i++) { w_prime_screen_control[i] = w_prime_screen_control[i + 1]; };
	w_prime_screen_control[w_size-1] = lut_digits[(cn_flag << 4) | accumulator];
	w_prime_screen_control[w_size-1] |= static_cast<uint8_t>((!cn_flag) && m_flag_segment_decoder);
	last_w_update = THRESHOLD_CYCLE_UPDATE_W;
	cycle_curr_opcode += 2;
}

void SM5A::op_wr(){ 
	for (int i = 0; i < w_size-1; i++) { w_prime_screen_control[i] = w_prime_screen_control[i + 1]; };
	w_prime_screen_control[w_size-1] = accumulator & 0x07;
	last_w_update = THRESHOLD_CYCLE_UPDATE_W;
};

void SM5A::op_ws(){
	for (int i = 0; i < w_size-1; i++) { w_prime_screen_control[i] = w_prime_screen_control[i + 1]; };
	w_prime_screen_control[w_size-1] = accumulator | 8;
	last_w_update = THRESHOLD_CYCLE_UPDATE_W;
}

// input - output instructions
void SM5A::op_atr(){ r_output_control = accumulator & 0x0F; };

void SM5A::op_kta(){
	// Need to be more study
	// doubt whether the multiplexing index (the pin most often used for sound) should be shifted
	// In theorie, accumulator -> r_output_control -> pin multiplexing but accumulator directly seems to work better
	uint8_t pin_activate = (~accumulator) & 0x0F;/*(~r_output_control) & 0x0F;*/ // 0 -> Activate R_i ouput, 1-> desactivate R_i ouput
    debug_multiplexage_activate = pin_activate;
	
	accumulator = 0x00;
    for(int i = 0; i < 8; i++){ 
        if( (((pin_activate >> i) & 0x01) == 0x01) || input_no_multiplex){ // multiplexage by S
            accumulator = accumulator | k_input[i];
        }
    }
	debug_value_read_input = accumulator;
};

// Divider
void SM5A::op_dta(){ // sure???
	accumulator = (f_clock_divider >> 11) & 0xf;
};

// BIT manipulation

void SM5A::op_rmf(){ 
	m_flag_segment_decoder = 0x00;
	accumulator = 0x00;
};

void SM5A::op_smf(){ 	
	m_flag_segment_decoder = 0x01;
};

void SM5A::op_comcb(){ 
	cb_debordement_rom_program_counter ^= 1;
};

// other
void SM5A::op_comcn(){ 	
	cn_flag = !cn_flag;
};

void SM5A::op_skip(){}; // no instruction, program counter +1
