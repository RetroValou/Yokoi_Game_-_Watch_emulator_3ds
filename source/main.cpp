#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <tex3ds.h>
#include <string.h>
#include "vshader_shbin.h"
#include <vector>
#include <malloc.h>

#include "std/timer.h"
#include "std/load_file.h"
#include "std/GW_ROM.h"

#include "SM5XX/SM5XX.h"
#include "SM5XX/SM510/SM510.h"
#include "SM5XX/SM511_SM512/SM511_2.h"
#include "SM5XX/SM5A/SM5A.h"

#include "virtual_i_o/3ds_screen.h"
#include "virtual_i_o/3ds_sound.h"
#include "virtual_i_o/virtual_input.h"

const float _3DS_FPS_SCREEN_ = 60;


bool get_cpu(SM5XX*& cpu, const uint8_t* rom, uint16_t size_rom){
    if(size_rom == 1856){
        cpu = new SM5A();
        return true;
    }
    else if (size_rom == 4096)
    {
        for(int i = 0; i<16; i++){
            if(rom[i+704] != 0x00){ 
                cpu = new SM511_2();
                return true;
            } // SM511 game work with SM512
        }
        cpu = new SM510(); return true;
    }
    return false;
}

uint8_t index_game = 0;


void update_credit(Virtual_Screen* v_screen){
    std::string text = "Credits";

    v_screen->delete_all_text();
    v_screen->delete_all_img();
    
    text = std::string("<")+text+std::string(">");
    int16_t pos_x = (400 - text.length()*16)/2;
    int16_t pos_y = (240 - 16)/2;
    v_screen->set_text(text, pos_x, pos_y, 0, 2);

    v_screen->set_text("Developed by RetroValou ~", 60, 40, 1, 1);
    v_screen->set_text("Check out my game on Steam !", 46, 50, 1, 1);
    uint16_t info[7] = { 256, 64, 0, 21, 146, 43, 0}; 
    v_screen->set_img("romfs:/gfx/logo_pioupiou.t3x", info, 85, 66, 0);

    v_screen->set_text("Code inspired by Mame", 74, 135, 1, 1);
    v_screen->set_text("G&W for FPGAs (Adam Gastineau)", 42, 145, 1, 1);
    v_screen->set_text("and official sharp doc", 70, 155, 1, 1);
    v_screen->set_text("Thank's for all !", 90, 175, 1, 1);

    //v_screen->set_text("", 10, 205, 1, 1);
    v_screen->set_text("Public domain  Free to use", 10, 215, 1, 1);
    v_screen->set_text("No attribution required :)", 10, 225, 1, 1);

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen->update_text();
    v_screen->update_img(false);
    C3D_FrameEnd(0);
}


void update_name_game(Virtual_Screen* v_screen, bool for_choose = true){
    std::string text = get_name(index_game); if(text.empty()) { text = "_not_valid_"; }
    std::string path_console = get_path_console_img(index_game);
    const uint16_t* info = get_info_console_img(index_game);
    std::string date = get_date(index_game); if(text.empty()) { text = "_not_valid_"; }

    v_screen->delete_all_text();
    
    // Check if text contains brackets for two-line display
    std::string line1 = text;
    std::string line2 = "";
    size_t bracket_pos = text.find('(');
    if (bracket_pos != std::string::npos) {
        line1 = text.substr(0, bracket_pos);
        // Remove trailing space if present
        while (!line1.empty() && line1.back() == ' ') {
            line1.pop_back();
        }
        line2 = text.substr(bracket_pos);
    }
    
    if(!for_choose){ 
        line1 = std::string(" ") + line1;
    }
    else { 
        line1 = std::string("<") + line1 + std::string(">"); 
    }
    
    int16_t pos_x = (400 - line1.length()*16)/2;
    int16_t pos_y = (240 - 16)/2;
    
    // Always display two lines for consistent layout
    pos_y -= 16;
    v_screen->set_text(line1, pos_x, pos_y, 0, 2);
    
    int16_t pos_x2 = (400 - line2.length()*16)/2;
    v_screen->set_text(line2, pos_x2, pos_y + 32, 0, 2);
    
    v_screen->set_text(date, 160, pos_y+70, 0, 1);

    v_screen->set_text("L+R", 280, 228, 1, 1);
    v_screen->set_text("MENU", 276, 220, 1, 1);

    pos_x = (320 - info[4])/2;
    pos_y = (240 - info[5])/2;
    v_screen->set_img(path_console, info, pos_x, pos_y, 0);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen->update_img();
    v_screen->update_text();
    C3D_FrameEnd(0);
}


bool select_game(Virtual_Screen* v_screen){
    hidScanInput();
    u32 kDown = hidKeysHeld();

    if(kDown&KEY_DRIGHT){
        index_game = (index_game+1)%(get_nb_name()+1);

        if(index_game >= get_nb_name()){ update_credit(v_screen); }
        else { update_name_game(v_screen); }
        sleep_us_p(300000);
    }
    else if(kDown&KEY_DLEFT){
        if(index_game == 0){ index_game = (get_nb_name()+1);}
        index_game = index_game-1;

        if(index_game >= get_nb_name()){ update_credit(v_screen); }
        else { update_name_game(v_screen); }
        sleep_us_p(300000);
    }

    if(index_game >= get_nb_name()){ return false; } // credit

    if( (kDown&KEY_A) || (kDown&KEY_B) || (kDown&KEY_START) ||
        (kDown&KEY_Y) || (kDown&KEY_X) ) { return true; }

    return false;
}

void input_get(Virtual_Input* v_input){
	hidScanInput();
	u32 kDown = hidKeysHeld();

    v_input->set_input(PART_SETUP, BUTTON_TIME, kDown&KEY_L);
    v_input->set_input(PART_SETUP, BUTTON_GAMEA, kDown&KEY_START);
    v_input->set_input(PART_SETUP, BUTTON_GAMEB, kDown&KEY_SELECT);

    if(v_input->left_configuration == CONF_1_BUTTON_ACTION){
        bool check;
        if(v_input->right_configuration == CONF_1_BUTTON_ACTION)
            { check = (kDown&KEY_DUP)||(kDown&KEY_DDOWN)||(kDown&KEY_DLEFT)||(kDown&KEY_Y); }
        else { check = (kDown&KEY_DUP)||(kDown&KEY_DDOWN)||(kDown&KEY_DLEFT)||(kDown&KEY_DRIGHT); }
        v_input->set_input(PART_LEFT, BUTTON_ACTION, check);
    }
    else {
        v_input->set_input(PART_LEFT, BUTTON_LEFT, kDown&KEY_DLEFT);
        v_input->set_input(PART_LEFT, BUTTON_RIGHT, kDown&KEY_DRIGHT);
        v_input->set_input(PART_LEFT, BUTTON_UP, kDown&KEY_DUP);
        v_input->set_input(PART_LEFT, BUTTON_DOWN, kDown&KEY_DDOWN);
    }

    if(v_input->right_configuration == CONF_1_BUTTON_ACTION){
        bool check;
        if(v_input->left_configuration == CONF_1_BUTTON_ACTION)
            { check = (kDown&KEY_A)||(kDown&KEY_B)||(kDown&KEY_X)||(kDown&KEY_DRIGHT); }
        else { check = (kDown&KEY_A)||(kDown&KEY_B)||(kDown&KEY_X)||(kDown&KEY_Y); }
        v_input->set_input(PART_RIGHT, BUTTON_ACTION, check);
    }
    else {
        v_input->set_input(PART_RIGHT, BUTTON_LEFT, kDown&KEY_Y);
        v_input->set_input(PART_RIGHT, BUTTON_RIGHT, kDown&KEY_A);
        v_input->set_input(PART_RIGHT, BUTTON_UP, kDown&KEY_X);
        v_input->set_input(PART_RIGHT, BUTTON_DOWN, kDown&KEY_B);
    }
}



void init_game(SM5XX** cpu, Virtual_Screen* v_screen, Virtual_Sound* v_sound, Virtual_Input** v_input){
    v_screen->Quit_Game();
    v_sound->Quit_Game();

    const GW_rom* game = load_game(index_game);


    v_screen->load_visual(game->path_segment
                        , game->segment, game->size_segment, game->segment_info
                        , game->path_background, game->background_info);
    v_screen->init_visual();

    get_cpu(*cpu, game->rom, game->size_rom);
    (*cpu)->init();
    (*cpu)->load_rom(game->rom, game->size_rom);
    (*cpu)->load_rom_melody(game->melody, game->size_melody);


    v_sound->initialize((*cpu)->frequency, (*cpu)->sound_divide_frequency, _3DS_FPS_SCREEN_);
    v_sound->play_sample();

    (*v_input) = get_input_config((*cpu), game->ref);
}



enum GameState {
    STATE_MENU,
    STATE_PLAY
};


int main()
{
	Virtual_Screen v_screen;
    Virtual_Sound v_sound;
    Virtual_Input* v_input;
    SM5XX* cpu = nullptr;

    v_screen.config_screen();
    v_sound.configure_sound();

    uint32_t curr_rate = 0;

    GameState state = STATE_MENU;
    update_name_game(&v_screen);

    while (aptMainLoop())
	{
        switch (state)
        {
            case STATE_MENU:
                if(select_game(&v_screen)){
                    init_game(&cpu, &v_screen, &v_sound, &v_input);
                    state = STATE_PLAY;
                    curr_rate = 0; // for compense fractional step cpu
                } 
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C3D_FrameEnd(0);
                break;
            case STATE_PLAY:
                v_sound.play_sample();
                input_get(v_input);
                curr_rate += cpu->frequency;
                uint32_t step = curr_rate/_3DS_FPS_SCREEN_;
                curr_rate -= step*_3DS_FPS_SCREEN_;

                while(step > 0) { 
                    if(cpu->step()){ v_screen.update_buffer_video(cpu); }
                    v_sound.update_sound(cpu); 
                    step -= 1;
                }
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                v_screen.update_screen();
                C3D_FrameEnd(0);

                if((hidKeysHeld()&(KEY_L|KEY_R)) == (KEY_L|KEY_R)){
                    state = STATE_MENU;
                    update_name_game(&v_screen);
                }
                break;
        }
    }

    v_sound.Exit();
	v_screen.Exit();
	return 0;
}