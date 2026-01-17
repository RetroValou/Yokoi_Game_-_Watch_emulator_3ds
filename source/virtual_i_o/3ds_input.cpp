#include "virtual_i_o/3ds_input.h"


void Input_Manager_3ds::input_Update()
{
    hidScanInput();
    kDown = hidKeysDown();
    kHeld = hidKeysHeld();
    kUp   = hidKeysUp();
}


bool Input_Manager_3ds::input_isHeld(u32 input){
    if((kHeld&input) == input){
        last_input = input;
        return true;
    }
    return false;
};


bool Input_Manager_3ds::input_justPressed(u32 input, bool protect_bad){
    bool result = (kDown&input) == input;

    if(result){
        if(input != last_input){
            last_input = input;
            last_time_input = time_us_64_p();
            return true;
        }
        last_input = input;
        if(time_us_64_p() - last_time_input > PROTECT_BAD_INPUT
                    || time_us_64_p() < last_time_input // if time reset
                    || !protect_bad) 
        {
            last_time_input = time_us_64_p();
            return true;   
        }
    }
    return false;
}



bool Input_Manager_3ds::input_Held_Increase(u32 input, uint64_t time_between){
    if(input_justPressed(input)){ return true; }
    //return false;

    bool result = (kHeld&input) == input;

    if(result){
        if(input != last_input){
            last_input = input;
            last_time_input = time_us_64_p();
            return true;
        }
        last_input = input;
        
        if(time_us_64_p() - last_time_input > time_between
                    || time_us_64_p() < last_time_input ) // if time reset
        {
            last_time_input = time_us_64_p();
            return true;   
        }
    }
    return false;
}



void Input_Manager_3ds::input_GW_Update(Virtual_Input* v_input){

    v_input->set_input(PART_SETUP, BUTTON_TIME, kHeld&KEY_L);
    v_input->set_input(PART_SETUP, BUTTON_GAMEA, kHeld&KEY_START);
    v_input->set_input(PART_SETUP, BUTTON_GAMEB, kHeld&KEY_SELECT);

    if(v_input->left_configuration == CONF_1_BUTTON_ACTION){
        bool check;
        if(v_input->right_configuration == CONF_1_BUTTON_ACTION)
            { check = (kHeld&KEY_DUP)||(kHeld&KEY_DDOWN)||(kHeld&KEY_DLEFT)||(kHeld&KEY_Y); }
        else { check = (kHeld&KEY_DUP)||(kHeld&KEY_DDOWN)||(kHeld&KEY_DLEFT)||(kHeld&KEY_DRIGHT); }
        v_input->set_input(PART_LEFT, BUTTON_ACTION, check);
    }
    else {
        v_input->set_input(PART_LEFT, BUTTON_LEFT, kHeld&KEY_DLEFT);
        v_input->set_input(PART_LEFT, BUTTON_RIGHT, kHeld&KEY_DRIGHT);
        v_input->set_input(PART_LEFT, BUTTON_UP, kHeld&KEY_DUP);
        v_input->set_input(PART_LEFT, BUTTON_DOWN, kHeld&KEY_DDOWN);
    }

    if(v_input->right_configuration == CONF_1_BUTTON_ACTION){
        bool check;
        if(v_input->left_configuration == CONF_1_BUTTON_ACTION)
            { check = (kHeld&KEY_A)||(kHeld&KEY_B)||(kHeld&KEY_X)||(kHeld&KEY_DRIGHT); }
        else { check = (kHeld&KEY_A)||(kHeld&KEY_B)||(kHeld&KEY_X)||(kHeld&KEY_Y); }
        v_input->set_input(PART_RIGHT, BUTTON_ACTION, check);
    }
    else {
        v_input->set_input(PART_RIGHT, BUTTON_LEFT, kHeld&KEY_Y);
        v_input->set_input(PART_RIGHT, BUTTON_RIGHT, kHeld&KEY_A);
        v_input->set_input(PART_RIGHT, BUTTON_UP, kHeld&KEY_X);
        v_input->set_input(PART_RIGHT, BUTTON_DOWN, kHeld&KEY_B);
    }
}

