#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <tex3ds.h>
#include <string.h>
#include "vshader_shbin.h"
#include <vector>
#include <malloc.h>
#include <time.h>
#include <stdio.h>

#include "std/timer.h"
#include "std/load_file.h"
#include "std/GW_ROM.h"
#include "std/settings.h"
#include "std/savestate.h"
#include "std/gw_pack.h"
#include "std/platform_paths.h"
#include "std/debug_log.h"

#include "SM5XX/SM5XX.h"
#include "SM5XX/SM510/SM510.h"
#include "SM5XX/SM511_SM512/SM511_2.h"
#include "SM5XX/SM5A/SM5A.h"

#include "virtual_i_o/3ds_screen.h"
#include "virtual_i_o/3ds_sound.h"
#include "virtual_i_o/virtual_input.h"
#include "virtual_i_o/3ds_input.h"


const float _3DS_FPS_SCREEN_ = 60;
const int _INPUT_SETTING_ = (KEY_L|KEY_B);
const int _INPUT_SETTING_OTHER_ = (KEY_ZL|KEY_ZR);

const int _INPUT_MENU_ = (KEY_L|KEY_R);

const uint64_t _TIME_MOVE_MENU_ = 400000;
const uint64_t _TIME_MOVE_VALUE_SETTING_ = 300000;



uint8_t index_game = 0;

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


static std::string g_pack_load_error;

static constexpr const char* k3dsRomPackPath = "sdmc:/3ds/yokoi_pack_3ds.ykp";

#if !defined(YOKOI_EMBEDDED_ASSETS)
static void show_pack_required_console(const std::string& err) {
    gfxInitDefault();

    PrintConsole top;
    PrintConsole bottom;
    consoleInit(GFX_TOP, &top);
    consoleInit(GFX_BOTTOM, &bottom);

    consoleSelect(&top);
    printf("ROM pack missing/outdated\n\n");
    printf("Place this file at:\n%s\n\n", k3dsRomPackPath);
    if (!err.empty()) {
        printf("Reason:\n%s\n\n", err.c_str());
    }
    printf("Press START to exit.\n");

    consoleSelect(&bottom);
    printf("Copy the pack, then restart the app.\n");

    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START) {
            break;
        }
        gspWaitForVBlank();
    }

    gfxExit();
}
#endif // !defined(YOKOI_EMBEDDED_ASSETS)

static void show_pack_required_screen(Virtual_Screen& v_screen, const std::string& err) {
    v_screen.delete_all_text();
    v_screen.delete_all_img();

    v_screen.set_text("ROM pack missing/outdated", 28, 88, 0, 2);
    v_screen.set_text("Place file at:", 40, 128, 0, 1);
    v_screen.set_text(k3dsRomPackPath, 8, 144, 0, 1);
    if (!err.empty()) {
        v_screen.set_text("Reason:", 40, 168, 0, 1);
        v_screen.set_text(err, 8, 184, 0, 1);
    }
    v_screen.set_text("Restart app after copying", 28, 212, 0, 1);

    v_screen.set_text("Press START to exit", 52, 110, 1, 1);
    v_screen.set_text("(then copy pack and restart)", 16, 130, 1, 1);
 
    // Render once, then idle until exit.
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen.update_img();
    v_screen.update_text();
    C3D_FrameEnd(0);

    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_START) {
            break;
        }
        gspWaitForVBlank();
    }
}

static void show_start_game_error(Virtual_Screen& v_screen, const std::string& msg) {
    v_screen.delete_all_text();
    v_screen.delete_all_img();

    v_screen.set_text("Failed to start game", 40, 90, 0, 2);
    if (!msg.empty()) {
        v_screen.set_text(msg, 8, 130, 0, 1);
    }
    v_screen.set_text("Press B to return", 56, 200, 0, 1);

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen.update_img();
    v_screen.update_text();
    C3D_FrameEnd(0);

    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & (KEY_B | KEY_START)) {
            break;
        }
        gspWaitForVBlank();
    }
}

void set_time_cpu(SM5XX* cpu) {
    if (!cpu->is_time_set()) {
        // Get the current time from the 3DS RTC (Real-Time Clock)
        time_t currentTime = time(NULL);
        struct tm *timeStruct = localtime(&currentTime);

        // Set the time on the emulated CPU
        if (timeStruct != nullptr) {
            cpu->set_time(timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
            cpu->time_set(true); // inform cpu that time is set
        }
    }
}

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
    // If we have no games at all (pack-only build without a pack), show a clear message.
    if (get_nb_name() == 0) {
        v_screen->delete_all_text();
        v_screen->set_text("<No games>", 120, 80, 0, 2);
        v_screen->set_text("ROM pack not found", 68, 120, 0, 1);
        v_screen->set_text("sdmc:/3ds/yokoi_pack_3ds.ykp", 12, 136, 0, 1);
        if (!g_pack_load_error.empty()) {
            v_screen->set_text(g_pack_load_error, 12, 156, 0, 1);
        }
        return;
    }

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
}


void update_text_indicator(Virtual_Screen* v_screen){
    v_screen->set_text("L+R", 280, 228, 1, 1);
    v_screen->set_text("MENU", 276, 220, 1, 1);
    
    bool new3DS = false;
    Result res = APT_CheckNew3DS(&new3DS);
    if(R_SUCCEEDED(res)) { // Only visual, two value work
        if(new3DS) { // New 3ds -> show ZL+ZR
            v_screen->set_text("ZL+ZR", 10, 228, 1, 1); 
        } 
        else { // Old 3ds -> show L+B
            v_screen->set_text("L+B", 10, 228, 1, 1); 
        } 
    }
    else {
        v_screen->set_text("L+B", 10, 228, 1, 1); 
    }
    
    v_screen->set_text("SETTINGS", 2, 220, 1, 1);

    #if defined(YOKOI_SHOW_MSG_ROM)    
        // Surface pack load failures in the UI so missing packs are obvious.
        if (!gw_pack::is_loaded() && !g_pack_load_error.empty()) {
            v_screen->set_text("ROM pack load failed:", 4, 200, 0, 1);
            v_screen->set_text("sdmc:/3ds/yokoi_pack_3ds.ykp", 4, 212, 0, 1);
        }
    #endif
}



void update_name_game_bottom(Virtual_Screen* v_screen){
    std::string path_console = get_path_console_img(index_game);
    const uint16_t* info = get_info_console_img(index_game);

    if (path_console.empty() || info == nullptr) {
        return;
    }
    
    int16_t pos_x = (320 - info[4])/2;
    int16_t pos_y = (240 - info[5])/2;
    v_screen->set_img(path_console, info, pos_x, pos_y, 0);

    update_text_indicator(v_screen);
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
int handle_menu_input(Virtual_Screen* v_screen, Input_Manager_3ds* input_manager){

    if(input_manager->input_Held_Increase(KEY_DRIGHT, _TIME_MOVE_MENU_)){
        index_game = (index_game+1)%(get_nb_name()+1);

        if(index_game >= get_nb_name()){ update_credit(v_screen); }
        else { 
            update_name_game(v_screen);
            save_last_game(get_name(index_game)); // Save the selected game
        }
    }
    else if(input_manager->input_Held_Increase(KEY_DLEFT, _TIME_MOVE_MENU_)){
        if(index_game == 0){ index_game = (get_nb_name()+1);}
        index_game = index_game-1;

        if(index_game >= get_nb_name()){ update_credit(v_screen); }
        else { 
            update_name_game(v_screen);
            save_last_game(get_name(index_game)); // Save the selected game
        }
    }

    // Settings accessed via ZL+ZR buttons (only on press, not held)
    if(input_manager->input_isHeld(_INPUT_SETTING_)
        || input_manager->input_isHeld(_INPUT_SETTING_OTHER_)) {
        return 2; // Go to settings
    }

    if(index_game >= get_nb_name()){ return 0; } // credit, stay in menu

    // Use kDown for action buttons to only trigger once per press
    if( input_manager->input_justPressed(KEY_A) 
        /*|| input_manager->input_justPressed(KEY_B)*/ 
        || input_manager->input_justPressed(KEY_START) 
        || input_manager->input_justPressed(KEY_Y)
        || input_manager->input_justPressed(KEY_X) ) 
    { return 1; } // Start game

    return 0; // Stay in menu
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


void load_screen(Virtual_Screen* v_screen){
    v_screen->set_text("Loading...", 60, 100, 0, 2);
    v_screen->set_text("Please wait", 160, 140, 0, 2);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    v_screen->update_text();
    v_screen->update_img(false);
    C3D_FrameEnd(0);
}



bool init_game(SM5XX** cpu, Virtual_Screen* v_screen, Virtual_Sound* v_sound, Virtual_Input** v_input, bool load_save){
    YOKOI_LOG("init_game: start index=%u load_save=%d", (unsigned)index_game, load_save ? 1 : 0);
    
    load_screen(v_screen);

    v_screen->Quit_Game();
    v_sound->Quit_Game();

    // NOTE: Do not delete *cpu or *v_input here.
    // On this codebase SM5XX and Virtual_Input do not guarantee virtual destructors,
    // so deleting via base pointer is undefined behavior and can crash on real hardware.
    // We'll leak during debugging; once we identify the crash point we can refactor ownership safely.

    const GW_rom* game = load_game(index_game);
    YOKOI_LOG("init_game: load_game -> %p", (const void*)game);

    if (!game) {
        YOKOI_LOG("init_game: ERROR no game data");
        show_start_game_error(*v_screen, "No game data");
        return false;
    }

    if (!game->rom || game->size_rom == 0) {
        YOKOI_LOG("init_game: ERROR missing rom ptr=%p size=%u", (const void*)game->rom, (unsigned)game->size_rom);
        show_start_game_error(*v_screen, "ROM missing");
        return false;
    }

    YOKOI_LOG("init_game: load_visual seg='%s' bg='%s'", game->path_segment.c_str(), game->path_background.c_str());
    if (!v_screen->load_visual(game->path_segment
                        , game->segment, game->size_segment, game->segment_info
                        , game->path_background, game->background_info)) {
        YOKOI_LOG("init_game: ERROR load_visual failed");
        show_start_game_error(*v_screen, "Texture load failed");
        return false;
    }
    YOKOI_LOG("init_game: init_visual");
    if (!v_screen->init_visual()) {
        YOKOI_LOG("init_game: ERROR init_visual failed");
        show_start_game_error(*v_screen, "Visual init failed");
        return false;
    }
    
    // Reapply user settings after visual initialization
    v_screen->refresh_settings();

    YOKOI_LOG("init_game: get_cpu size=%u", (unsigned)game->size_rom);
    if (!get_cpu(*cpu, game->rom, game->size_rom) || !*cpu) {
        YOKOI_LOG("init_game: ERROR get_cpu failed");
        show_start_game_error(*v_screen, "Unsupported ROM");
        return false;
    }
    YOKOI_LOG("init_game: cpu=%p", (const void*)*cpu);
    (*cpu)->init();
    YOKOI_LOG("init_game: cpu init ok");
    (*cpu)->load_rom(game->rom, game->size_rom);
    (*cpu)->load_rom_melody(game->melody, game->size_melody);
    (*cpu)->load_rom_time_addresses(game->ref);
    YOKOI_LOG("init_game: cpu rom loaded ref='%s'", game->ref.c_str());

    // Load saved state if requested
    if(load_save) {
        YOKOI_LOG("init_game: loading save state");
        load_game_state(*cpu, index_game);
    }

    //(*cpu)->debug_dump_ram_state("last_ram_state_before_load.txt");

    v_sound->initialize((*cpu)->frequency, (*cpu)->sound_divide_frequency, _3DS_FPS_SCREEN_);
    v_sound->play_sample();
    YOKOI_LOG("init_game: sound init ok");

    (*v_input) = get_input_config((*cpu), game->ref);
    if (!*v_input) {
        YOKOI_LOG("init_game: ERROR input config missing for ref='%s'", game->ref.c_str());
        show_start_game_error(*v_screen, "Input config missing");
        return false;
    }
    YOKOI_LOG("init_game: input config ok (%p)", (const void*)*v_input);

    set_time_cpu(*cpu);
    YOKOI_LOG("init_game: success");

    //(*cpu)->debug_dump_ram_state("post_time_set.txt");

    return true;
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



bool handle_settings_input(Virtual_Screen* v_screen, Input_Manager_3ds* input_manager) {

    // Navigate between settings
    if (input_manager->input_Held_Increase(KEY_DUP, _TIME_MOVE_MENU_)) {
        selected_setting = (selected_setting - 1 + NUM_SETTINGS) % NUM_SETTINGS;
        update_settings_display(v_screen);
    }
    else if (input_manager->input_Held_Increase(KEY_DDOWN, _TIME_MOVE_MENU_)) {
        selected_setting = (selected_setting + 1) % NUM_SETTINGS;
        update_settings_display(v_screen);
    }
    

    // Time wait before change value
    uint64_t time_check = 0;
    switch (selected_setting) {
        case 0: // Background color preset
            time_check = _TIME_MOVE_MENU_;
            break;
        
        case 1: // Segment marking alpha
            time_check = _TIME_MOVE_VALUE_SETTING_;
            break;
    }

    if (input_manager->input_Held_Increase(KEY_DRIGHT, time_check)) {
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
    }
    else if (input_manager->input_Held_Increase(KEY_DLEFT, time_check)) {
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
    }
    
    // Save and return
    if (input_manager->input_justPressed(KEY_A)) {
        save_settings();
        return true; // Exit settings
    }
    
    // Cancel (don't save)
    if (input_manager->input_justPressed(KEY_B)) {
        load_settings(); // Reload original settings
        return true; // Exit settings
    }
    
    // Reset to defaults
    if (input_manager->input_justPressed(KEY_X)) {
        reset_settings_to_default();
        update_settings_display(v_screen);
        sleep_us_p(200000);
    }
    
    return false;
}


int main()
{
	debug_log::init();
    YOKOI_LOG("main: start");
	Virtual_Screen v_screen;
    Virtual_Sound v_sound;
    Virtual_Input* v_input;
    Input_Manager_3ds input_manager;
    SM5XX* cpu = nullptr;

    // Try loading the external ROM pack first.
    // In pack-only builds, show a console-based blocking screen if it's missing/outdated.
    // (This avoids relying on romfs textures/font rendering, which may not be present.)
    bool pack_ok = false;
    {
		YOKOI_LOG("main: load pack '%s'", k3dsRomPackPath);
        std::string err;
        pack_ok = gw_pack::load(k3dsRomPackPath, &err);
		YOKOI_LOG("main: pack_ok=%d err='%s'", pack_ok ? 1 : 0, err.c_str());
        if (!pack_ok && !err.empty()) {
            g_pack_load_error = err;
            printf("ROM pack load failed: %s\n", err.c_str());
        } else {
            g_pack_load_error.clear();
        }

#if !defined(YOKOI_EMBEDDED_ASSETS)
        if (!pack_ok) {
            show_pack_required_console(g_pack_load_error);
            return 0;
        }
#endif
    }

    YOKOI_LOG("main: config_screen");
    v_screen.config_screen();
    YOKOI_LOG("main: config_screen ok");
    v_sound.configure_sound();
    YOKOI_LOG("main: configure_sound ok");

    // Load settings on startup
    load_settings();
	YOKOI_LOG("main: settings loaded");

#if defined(YOKOI_SHOW_MSG_ROM)    
    // If pack load failed but we're not pack-only, surface it in-app.
    if (!pack_ok && !g_pack_load_error.empty()) {
        show_pack_required_screen(v_screen, g_pack_load_error);
    }
#endif

    // Load the last selected game index
    index_game = load_last_game_index();

    // If the current pack has fewer games than when settings were saved, default to first game.
    {
        const size_t n = get_nb_name();
        if (n == 0) {
            index_game = 0;
        } else if (index_game >= (uint8_t)n) {
            index_game = 0;
        }
    }

    uint32_t curr_rate = 0;

    GameState state = STATE_MENU;
    GameState previous_state = STATE_MENU; // Track where we came from before settings
    bool just_exited_settings = false; // Prevent immediate input after exiting settings
    update_name_game(&v_screen);
	YOKOI_LOG("main: menu shown");

    // Add a grace period for setting the time on CPU after game start
    const int TIME_SET_GRACE_PERIOD = 500;
    int time_set_grace_counter = TIME_SET_GRACE_PERIOD;

    while (aptMainLoop())
	{
        input_manager.input_Update();

        switch (state)
        {
            case STATE_MENU:
                {
                    if(just_exited_settings) {
                        // Skip input processing for one frame after exiting settings
                        just_exited_settings = false;
                    }
                    else {
                        int menu_result = handle_menu_input(&v_screen, &input_manager);
                        if(menu_result == 1) {
                            // Check if save exists, if so go to prompt, otherwise start fresh
                            if(save_state_exists(index_game)) {
                                state = STATE_SAVE_PROMPT;
                            } else {
                                if (init_game(&cpu, &v_screen, &v_sound, &v_input, false)) {
                                    state = STATE_PLAY;
                                    curr_rate = 0;
                                    time_set_grace_counter = TIME_SET_GRACE_PERIOD; // Set time for first N cycles
                                } else {
                                    state = STATE_MENU;
                                    update_name_game(&v_screen);
                                }
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
                    

                    bool should_start = false;
                    bool load_save = false;
                    
                    if(input_manager.input_justPressed(KEY_A)) {
                        should_start = true;
                        load_save = true;
                    }
                    else if(input_manager.input_justPressed(KEY_B)) {
                        should_start = true;
                        load_save = false;
                    }
                    
                    if(should_start) {
                        v_screen.delete_all_text();
                        if (init_game(&cpu, &v_screen, &v_sound, &v_input, load_save)) {
                            state = STATE_PLAY;
                            curr_rate = 0;
                            time_set_grace_counter = TIME_SET_GRACE_PERIOD; // Set time for first N cycles
                            restore_single_screen_console(&v_screen);
                        } else {
                            state = STATE_MENU;
                            update_name_game(&v_screen);
                        }
                    }

                }
                break;
            case STATE_PLAY:
                {
                    v_sound.play_sample();
                    input_manager.input_GW_Update(v_input);
                    curr_rate += cpu->frequency;
                    uint32_t step = curr_rate/_3DS_FPS_SCREEN_;
                    curr_rate -= step*_3DS_FPS_SCREEN_;

                    while(step > 0) { 
                        if(cpu->step()) { 
                            // Only set time for the first few cycles after game start, otherwise the CPU
                            // won't set the correct initial time from the 3DS RTC.
                            if (time_set_grace_counter > 0) {
                                time_set_grace_counter--;
                                // Call set_time_cpu to ensure time is set during the grace period
                                cpu->time_set(false); // Reset time set flag so the call is forced
                                set_time_cpu(cpu);
                            }
                            v_screen.update_buffer_video(cpu); 
                        }
                        v_sound.update_sound(cpu); 
                        step -= 1;
                    }

                    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                    v_screen.update_screen();
                    C3D_FrameEnd(0);

                    if(input_manager.input_isHeld(_INPUT_MENU_)){
                        // Save game state before exiting to menu
                        save_game_state(cpu, index_game);
                        state = STATE_MENU;
                        update_name_game(&v_screen);
                        cpu->time_set(false); // Reset time set flag
                    }
                    else if(input_manager.input_isHeld(_INPUT_SETTING_)
                            || input_manager.input_isHeld(_INPUT_SETTING_OTHER_)){
                        // Go to settings from gameplay
                        previous_state = STATE_PLAY;
                        state = STATE_SETTINGS;
                        update_settings_display(&v_screen);
                        sleep_us_p(200000); // Debounce
                        cpu->time_set(false); // Reset time set flag
                    }
                }
                break;
            case STATE_SETTINGS:
                {
                    if(handle_settings_input(&v_screen, &input_manager)) {
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