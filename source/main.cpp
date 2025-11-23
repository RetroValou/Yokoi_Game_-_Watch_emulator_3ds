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
#include "std/settings.h"
#include "std/savestate.h"

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


void update_name_game_top(Virtual_Screen* v_screen, bool for_choose = true){
    std::string text = get_name(index_game); if(text.empty()) { text = "_not_valid_"; }
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
    
    v_screen->set_text("ZL+ZR", 10, 228, 1, 1);
    v_screen->set_text("SETTINGS", 2, 220, 1, 1);
}

void update_name_game_bottom(Virtual_Screen* v_screen){
    std::string path_console = get_path_console_img(index_game);
    const uint16_t* info = get_info_console_img(index_game);
    
    int16_t pos_x = (320 - info[4])/2;
    int16_t pos_y = (240 - info[5])/2;
    v_screen->set_img(path_console, info, pos_x, pos_y, 0);
}

void update_name_game(Virtual_Screen* v_screen, bool for_choose = true){
    update_name_game_top(v_screen, for_choose);
    update_name_game_bottom(v_screen);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen->update_img();
    v_screen->update_text();
    C3D_FrameEnd(0);
}

// Returns: 0 = stay in menu, 1 = start game, 2 = go to settings
int handle_menu_input(Virtual_Screen* v_screen){
    hidScanInput();
    u32 kHeld = hidKeysHeld();
    u32 kDown = hidKeysDown();

    if(kHeld&KEY_DRIGHT){
        index_game = (index_game+1)%(get_nb_name()+1);

        if(index_game >= get_nb_name()){ update_credit(v_screen); }
        else { 
            update_name_game(v_screen);
            save_last_game(get_name(index_game)); // Save the selected game
        }
        sleep_us_p(300000);
    }
    else if(kHeld&KEY_DLEFT){
        if(index_game == 0){ index_game = (get_nb_name()+1);}
        index_game = index_game-1;

        if(index_game >= get_nb_name()){ update_credit(v_screen); }
        else { 
            update_name_game(v_screen);
            save_last_game(get_name(index_game)); // Save the selected game
        }
        sleep_us_p(300000);
    }

    // Settings accessed via ZL+ZR buttons (only on press, not held)
    if((kDown & (KEY_ZL|KEY_ZR)) == (KEY_ZL|KEY_ZR)) {
        return 2; // Go to settings
    }

    if(index_game >= get_nb_name()){ return 0; } // credit, stay in menu

    // Use kDown for action buttons to only trigger once per press
    if( (kDown&KEY_A) || (kDown&KEY_B) || (kDown&KEY_START) ||
        (kDown&KEY_Y) || (kDown&KEY_X) ) { return 1; } // Start game

    return 0; // Stay in menu
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

void restore_single_screen_console(Virtual_Screen* v_screen) {
    if(v_screen->nb_screen == 1 || v_screen->is_double_in_one_screen()) {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        update_name_game_bottom(v_screen);
        v_screen->update_img();
        v_screen->update_text();
        C3D_FrameEnd(0);
    }
}

void init_game(SM5XX** cpu, Virtual_Screen* v_screen, Virtual_Sound* v_sound, Virtual_Input** v_input, bool load_save){
    v_screen->Quit_Game();
    v_sound->Quit_Game();

    const GW_rom* game = load_game(index_game);


    v_screen->load_visual(game->path_segment
                        , game->segment, game->size_segment, game->segment_info
                        , game->path_background, game->background_info);
    v_screen->init_visual();
    
    // Reapply user settings after visual initialization
    v_screen->refresh_settings();

    get_cpu(*cpu, game->rom, game->size_rom);
    (*cpu)->init();
    (*cpu)->load_rom(game->rom, game->size_rom);
    (*cpu)->load_rom_melody(game->melody, game->size_melody);

    // Load saved state if requested
    if(load_save) {
        load_game_state(*cpu, index_game);
    }


    v_sound->initialize((*cpu)->frequency, (*cpu)->sound_divide_frequency, _3DS_FPS_SCREEN_);
    v_sound->play_sample();

    (*v_input) = get_input_config((*cpu), game->ref);
}



enum GameState {
    STATE_MENU,
    STATE_PLAY,
    STATE_SETTINGS,
    STATE_SAVE_PROMPT
};
 

// Settings UI state
int selected_setting = 0;
int selected_bg_preset = 0;
const int NUM_SETTINGS = 2; // Background color and segment marking alpha

void update_settings_display(Virtual_Screen* v_screen) {
    v_screen->delete_all_text();
    v_screen->delete_all_img();

    int text_offset_x = 20;
    
    // Title
    v_screen->set_text("Settings", text_offset_x, 10, 0, 2);
    
    // Background Color setting
    std::string bg_text = "Background Color: ";
    for (int i = 0; i < NUM_BG_PRESETS; i++) {
        if (g_settings.background_color == BACKGROUND_PRESETS[i].color) {
            bg_text += BACKGROUND_PRESETS[i].name;
            selected_bg_preset = i;
            break;
        }
    }
    if (selected_setting == 0) {
        bg_text = "> " + bg_text + " <";
    }
    v_screen->set_text(bg_text, text_offset_x, 60, 0, 1);
    
    // Segment marking alpha setting
    char alpha_str[32];
    snprintf(alpha_str, sizeof(alpha_str), "Marking Effect: %d/255", g_settings.segment_marking_alpha);
    std::string alpha_text = alpha_str;
    if (selected_setting == 1) {
        alpha_text = "> " + alpha_text + " <";
    }
    v_screen->set_text(alpha_text, text_offset_x, 90, 0, 1);
    
    // Instructions
    v_screen->set_text("UP/DOWN: Select setting", text_offset_x, 150, 1, 1);
    v_screen->set_text("LEFT/RIGHT: Change value", text_offset_x, 160, 1, 1);
    v_screen->set_text("A: Save & Return", text_offset_x, 180, 1, 1);
    v_screen->set_text("B: Cancel", text_offset_x, 190, 1, 1);
    v_screen->set_text("X: Reset to defaults", text_offset_x, 200, 1, 1);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen->update_text();
    v_screen->update_img(false);
    C3D_FrameEnd(0);
}

bool handle_settings_input(Virtual_Screen* v_screen) {
    hidScanInput();
    u32 kDown = hidKeysDown();
    
    // Navigate between settings
    if (kDown & KEY_DUP) {
        selected_setting = (selected_setting - 1 + NUM_SETTINGS) % NUM_SETTINGS;
        update_settings_display(v_screen);
        sleep_us_p(150000);
    }
    else if (kDown & KEY_DDOWN) {
        selected_setting = (selected_setting + 1) % NUM_SETTINGS;
        update_settings_display(v_screen);
        sleep_us_p(150000);
    }
    
    // Change values
    else if (kDown & KEY_DRIGHT) {
        if (selected_setting == 0) {
            // Background color preset
            selected_bg_preset = (selected_bg_preset + 1) % NUM_BG_PRESETS;
            g_settings.background_color = BACKGROUND_PRESETS[selected_bg_preset].color;
        }
        else if (selected_setting == 1) {
            // Segment marking alpha
            g_settings.segment_marking_alpha = (g_settings.segment_marking_alpha + 1) % 256;
        }
        update_settings_display(v_screen);
        sleep_us_p(100000);
    }
    else if (kDown & KEY_DLEFT) {
        if (selected_setting == 0) {
            // Background color preset
            selected_bg_preset = (selected_bg_preset - 1 + NUM_BG_PRESETS) % NUM_BG_PRESETS;
            g_settings.background_color = BACKGROUND_PRESETS[selected_bg_preset].color;
        }
        else if (selected_setting == 1) {
            // Segment marking alpha
            g_settings.segment_marking_alpha = (g_settings.segment_marking_alpha - 1 + 256) % 256;
        }
        update_settings_display(v_screen);
        sleep_us_p(100000);
    }
    
    // Save and return
    if (kDown & KEY_A) {
        save_settings();
        return true; // Exit settings
    }
    
    // Cancel (don't save)
    if (kDown & KEY_B) {
        load_settings(); // Reload original settings
        return true; // Exit settings
    }
    
    // Reset to defaults
    if (kDown & KEY_X) {
        reset_settings_to_default();
        update_settings_display(v_screen);
        sleep_us_p(200000);
    }
    
    return false;
}


int main()
{
	Virtual_Screen v_screen;
    Virtual_Sound v_sound;
    Virtual_Input* v_input;
    SM5XX* cpu = nullptr;

    v_screen.config_screen();
    v_sound.configure_sound();

    // Load settings on startup
    load_settings();
    
    // Load the last selected game index
    index_game = load_last_game_index();

    uint32_t curr_rate = 0;

    GameState state = STATE_MENU;
    GameState previous_state = STATE_MENU; // Track where we came from before settings
    bool just_exited_settings = false; // Prevent immediate input after exiting settings
    update_name_game(&v_screen);

    while (aptMainLoop())
	{
        switch (state)
        {
            case STATE_MENU:
                {
                    if(just_exited_settings) {
                        // Skip input processing for one frame after exiting settings
                        just_exited_settings = false;
                    }
                    else {
                        int menu_result = handle_menu_input(&v_screen);
                        if(menu_result == 1) {
                            // Check if save exists, if so go to prompt, otherwise start fresh
                            if(save_state_exists(index_game)) {
                                state = STATE_SAVE_PROMPT;
                            } else {
                                init_game(&cpu, &v_screen, &v_sound, &v_input, false);
                                state = STATE_PLAY;
                                curr_rate = 0;
                            }
                        }
                        else if(menu_result == 2) {
                            // Go to settings
                            previous_state = STATE_MENU;
                            state = STATE_SETTINGS;
                            update_settings_display(&v_screen);
                        }
                    }
                    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                    C3D_FrameEnd(0);
                }
                break;
            case STATE_SAVE_PROMPT:
                {
                    // Display save state prompt
                    v_screen.delete_all_text();
                    v_screen.set_text("Load save state?", 80, 100, 1, 1);
                    v_screen.set_text("A: Load save", 80, 130, 1, 1);
                    v_screen.set_text("B: Start new game", 80, 150, 1, 1);
                    
                    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                    v_screen.update_text();
                    C3D_FrameEnd(0);
                    
                    hidScanInput();
                    u32 kDown = hidKeysDown();

                    bool should_start = false;
                    bool load_save = false;
                    
                    if(kDown & KEY_A) {
                        should_start = true;
                        load_save = true;
                    }
                    else if(kDown & KEY_B) {
                        should_start = true;
                        load_save = false;
                    }
                    
                    if(should_start) {
                        v_screen.delete_all_text();
                        init_game(&cpu, &v_screen, &v_sound, &v_input, load_save);
                        state = STATE_PLAY;
                        curr_rate = 0;
                        restore_single_screen_console(&v_screen);
                    }

                }
                break;
            case STATE_PLAY:
                {
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
                        // Save game state before exiting to menu
                        save_game_state(cpu, index_game);
                        state = STATE_MENU;
                        update_name_game(&v_screen);
                    }
                    else if((hidKeysHeld()&(KEY_ZL|KEY_ZR)) == (KEY_ZL|KEY_ZR)){
                        // Go to settings from gameplay
                        previous_state = STATE_PLAY;
                        state = STATE_SETTINGS;
                        update_settings_display(&v_screen);
                        sleep_us_p(200000); // Debounce
                    }
                }
                break;
            case STATE_SETTINGS:
                {
                    if(handle_settings_input(&v_screen)) {
                        // Exit settings back to previous state
                        state = previous_state;
                        if(state == STATE_MENU) {
                            just_exited_settings = true; // Prevent immediate input processing
                            update_name_game(&v_screen);
                        }
                        else if(state == STATE_PLAY) {
                            v_screen.delete_all_text();
                            v_screen.delete_all_img();

                            // Apply new settings
                            v_screen.refresh_settings();
                            restore_single_screen_console(&v_screen);
                        }
                    }
                    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                    C3D_FrameEnd(0);
                }
                break;
        }
    }

    v_sound.Exit();
	v_screen.Exit();
	return 0;
}