
#include "virtual_i_o/3ds_sound.h"
#include "std/timer.h"
#include <3ds.h>
#include <string.h>

constexpr float LIMIT_SQUARE = 0.8;

void Virtual_Sound::configure_sound(){
    ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

	float mix[12];
	memset(mix, 0, sizeof(mix));
	mix[0] = 1.0;
	mix[1] = 1.0;
	ndspChnSetMix(0, mix);
}

void Virtual_Sound::initialize(uint32_t v_freq, uint16_t v_divide_freq, float v_fps_screen){
    base_freq = v_freq;
    divide_freq = v_divide_freq;
    fps_screen = v_fps_screen;

    length_max_sequence = base_freq / (fps_screen*divide_freq) +1;    
    length_max_sequence += SEQUENCE_SECURITY_ADD;

    for(size_t i = 0; i < NB_BUFFER; i++){
        linearFree(buffer_sound[i]); buffer_sound[i] = nullptr;
        buffer_sound[i] = (int16_t*)linearAlloc(length_max_sequence*2 * sizeof(int16_t)); // 2 -> Stereo
        for(int s = 0; s < length_max_sequence; s++){ buffer_sound[i][s] = 0x00; }
        curr_size_buffer[i] = base_freq / (fps_screen*divide_freq);
    }

    curr_buffer = 0;
    curr_sequence = 0;
    curr_wait_before_update = 0;
    accu_freq_sequence = 0;
    curr_value = false;

	ndspChnSetRate(0, (base_freq/divide_freq)); 
    ndspChnSetPaused(0, false);
}


void Virtual_Sound::update_sound(SM5XX* cpu){
    curr_wait_before_update += 1;
    curr_value = curr_value || cpu->get_active_sound();
    if(curr_wait_before_update < divide_freq){ return; } // if divide_freq = 1 -> Always update
    curr_wait_before_update = 0;
    if(curr_sequence < curr_size_buffer[curr_buffer] && curr_sequence < length_max_sequence){
        buffer_sound[curr_buffer][2*curr_sequence] =  (curr_value ? int(INT16_MAX*LIMIT_SQUARE) : 0);
        buffer_sound[curr_buffer][2*curr_sequence+1] =  (curr_value ? int(INT16_MAX*LIMIT_SQUARE) : 0);
        curr_sequence += 1;
    }
    curr_value = false;
    //lissage_sound();
}

/*

void Virtual_Sound::lissage_sound(){
    uint32_t ind_study = (curr_sequence==0 && curr_buffer==0) ? (length_sequence * LENGTH_BUFFER-1) : (curr_sequence + curr_buffer*length_sequence - 1);

    uint32_t ind_before = (ind_study == 0) ? (length_sequence * LENGTH_BUFFER - 1) : (ind_study - 1);
    uint32_t ind_after  = (ind_study + 1) % (length_sequence * LENGTH_BUFFER);

    uint16_t voisins = (buffer_sound_activate[ind_before] + buffer_sound_activate[ind_after]) / 2;
    buffer_sound_activate[ind_study] = (buffer_sound_activate[ind_study] + voisins) / 2;
}
*/


void Virtual_Sound::play_sample(){
    uint32_t last_value = buffer_sound[curr_buffer][2*max(curr_sequence-1, 0)];
    while(curr_sequence < curr_size_buffer[curr_buffer] && curr_sequence < length_max_sequence){ 
        // ading last value for not make 'empty' value
        buffer_sound[curr_buffer][2*curr_sequence] =  last_value;
        buffer_sound[curr_buffer][2*curr_sequence+1] =  last_value;
        curr_sequence+=1;
    }
    //lissage_sound();

    waveBuf[curr_buffer].data_vaddr = buffer_sound[curr_buffer];
    waveBuf[curr_buffer].nsamples = curr_size_buffer[curr_buffer];
    DSP_FlushDataCache(buffer_sound[curr_buffer], curr_size_buffer[curr_buffer]*2* sizeof(int16_t));
    ndspChnWaveBufAdd(0, &waveBuf[curr_buffer]);

    curr_buffer = (curr_buffer+1)%NB_BUFFER;
    curr_sequence = 0;

    accu_freq_sequence = accu_freq_sequence + base_freq;
    curr_size_buffer[curr_buffer] = int(accu_freq_sequence / (fps_screen*divide_freq));
    accu_freq_sequence = accu_freq_sequence - curr_size_buffer[curr_buffer]*fps_screen*divide_freq;
}



void Virtual_Sound::Quit_Game(){
    ndspChnSetPaused(0, true);
    ndspChnWaveBufClear(0);
    svcSleepThread(5 * 100 * 1000);
    for(size_t i = 0; i < NB_BUFFER; i++){
        if (buffer_sound[i]) { linearFree(buffer_sound[i]); buffer_sound[i] = nullptr; }
    }
}

void Virtual_Sound::Exit(){
    Quit_Game();
    ndspExit();
}
