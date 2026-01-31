#pragma once
#include "SM5XX/SM5XX.h"
#include <cstdint>
#include <string>

constexpr uint8_t PART_SETUP = 0x00; // Game A, Time, acl, ...
constexpr uint8_t PART_LEFT = 0x01;
constexpr uint8_t PART_RIGHT = 0x02;

constexpr uint8_t CONF_NOTHING = 0x00;
constexpr uint8_t CONF_1_BUTTON_ACTION = 0x01;
constexpr uint8_t CONF_2_BUTTON_UPDOWN = 0x02;
constexpr uint8_t CONF_2_BUTTON_LEFTRIGHT = 0x03;
constexpr uint8_t CONF_4_BUTTON_DIRECTION = 0x04;

constexpr uint8_t BUTTON_NOTHING = 0x00;
constexpr uint8_t BUTTON_GAMEA = 0x01;
constexpr uint8_t BUTTON_GAMEB = 0x02;
constexpr uint8_t BUTTON_TIME = 0x03;
constexpr uint8_t BUTTON_ALARM = 0x04;
constexpr uint8_t BUTTON_ACL = 0x05;

constexpr uint8_t BUTTON_ACTION = 0x10;
constexpr uint8_t BUTTON_LEFT = 0x20;
constexpr uint8_t BUTTON_RIGHT = 0x30;
constexpr uint8_t BUTTON_UP = 0x40;
constexpr uint8_t BUTTON_DOWN = 0x50;


/*
    Multiplexage explain : 
    


*/



class Virtual_Input {
    public : 
        uint8_t left_configuration = CONF_NOTHING;
        uint8_t right_configuration = CONF_NOTHING;
        bool two_player = false;
        bool use_multiplexage = true;

        Virtual_Input(SM5XX* c) : cpu(c) {}   
        virtual ~Virtual_Input() = default; 
        virtual void set_input(uint8_t, uint8_t, bool, uint8_t = 1){};

    protected : 
        // Important: If multiple physical buttons map to the same K bit ("Down").
        // We must OR them together; otherwise an unpressed button will clear the bit
        // each frame and the game will never see it.
        struct OrKBit {
            bool primary_held = false;
            bool secondary_held = false;

            void set_primary(SM5XX* cpu, int group, int line, bool pressed) {
                primary_held = pressed;
                cpu->input_set(group, line, primary_held || secondary_held);
            }

            void set_secondary(SM5XX* cpu, int group, int line, bool pressed) {
                secondary_held = pressed;
                cpu->input_set(group, line, primary_held || secondary_held);
            }
        };

        SM5XX* cpu;
};


///////////////////////////// Game & Watch Input Configuration //////////////////////////////////////////////////////////

////// BALL : SM5A //////
class AC_01 : public Virtual_Input{
    public : 
        AC_01(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
            use_multiplexage = false;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Flagman : SM5A //////
class FL_02 : public Virtual_Input{
    public : 
        FL_02(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 0, state); break; // 1
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 3
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 2
                        case BUTTON_DOWN: cpu->input_set(2, 3, state); break; // 4
                        default: break; } break;
                default: break;
            }
        }
};


////// Vermin : SM5A //////
class MT_03 : public Virtual_Input{
    public : 
        MT_03(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
            use_multiplexage = false;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Fire (first version) / Kosmicheskiy most : SM5A //////
class RC_04 : public Virtual_Input{
    public : 
        RC_04(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
            use_multiplexage = false;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Judge : SM5A //////
class IP_05 : public Virtual_Input{
    public : 
        IP_05(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 1
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 3
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 2
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 4
                        default: break; } break;
                default: break;
            }
        }
};


////// Manhole : SM5A //////
class MH_06 : public Virtual_Input{
    public : 
        MH_06(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};


////// Helmet : SM5A //////
class CN_07 : public Virtual_Input{
    public : 
        CN_07(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Lion : SM5A //////
class LN_08 : public Virtual_Input{
    public : 
        LN_08(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};


////// Parachute : SM5A //////
class PR_21 : public Virtual_Input{
    public : 
        PR_21(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// octopus : SM5A //////
class OC_22 : public Virtual_Input{
    public : 
        OC_22(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Popeye : SM5A //////
class PP_23 : public Virtual_Input{
    public : 
        PP_23(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Chef : SM5A //////
class FP_24 : public Virtual_Input{
    public : 
        FP_24(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Mickey_Mouse/Egg/Ataka asteroidov/Biathlon/Circus/Hockey/Kosmicheskiy polyot/Kot-rybolov/
////// Kvaka-zadavaka/Morskaja ataka/Nochnye vorishki/Nu, pogodi!/Okhota/Razvedchiki kosmosa/
////// Vesyolye futbolisty : SM5A //////
class MC_25 : public Virtual_Input{
    public : 
        MC_25(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }


        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};


////// Fire (wide screen) : SM5A //////
class FR_27 : public Virtual_Input{
    public : 
        FR_27(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                default: break;
            }
        }
};

/////// Space Mission / Spider SM_11/SG_21 : SM5A //////
class SM_11 : public Virtual_Input{
    public : 
        SM_11(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 9, !state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 8, !state); break;
                        default: break; } break;
                default: break;
            }
        }
}; 

////// Super Goal Keeper SK_10 : SM5A //////
class SK_10 : public Virtual_Input{
    public : 
        SK_10(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Autoslalom IM_23 : SM5A //////
class IM_23 : public Virtual_Input{
    public : 
        IM_23(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(3, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(3, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(2, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Turtle Bridge : SM10 //////
class TL_28 : public Virtual_Input{
    public : 
        TL_28(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Fire Attack : SM10 //////
class ID_29 : public Virtual_Input{
    public : 
        ID_29(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Snoopy Tennis : SM10 //////
class SP_30 : public Virtual_Input{
    public : 
        SP_30(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Oil Panic : SM10 //////
class OP_51 : public Virtual_Input{
    public : 
        OP_51(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Donkey Kong : SM10 //////
class DK_52 : public Virtual_Input{
    public : 
        DK_52(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mickey & Donald : SM10 //////
class DM_53 : public Virtual_Input{
    public : 
        DM_53(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 0, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Green House : SM10 //////
class GH_54 : public Virtual_Input{
    public : 
        GH_54(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Donkey Kong  : SM10 //////
class JR_55 : public Virtual_Input{
    public : 
        JR_55(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario Bros  : SM10 //////
class MW_56 : public Virtual_Input{
    public : 
        MW_56(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Rain Shower  : SM10 //////
class LP_57 : public Virtual_Input{
    public : 
        LP_57(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(2, 0, state); break;
                        case BUTTON_UP: cpu->input_set(2, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(2, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(2, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Life Boat  : SM10 //////
class TC_58 : public Virtual_Input{
    public : 
        TC_58(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Pinball  : SM11 //////
class PB_59 : public Virtual_Input{
    public : 
        PB_59(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Black Jack  : SM12 //////
class BJ_60 : public Virtual_Input{
    public : 
        BJ_60(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 3, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Squish  : SM10 //////
class MG_61 : public Virtual_Input{
    public : 
        MG_61(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_LEFTRIGHT;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEB: cpu->input_set(1, 2, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 3, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Bomb Sweeper  : SM10 //////
class BD_62 : public Virtual_Input{
    public : 
        BD_62(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_NOTHING;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Safe Buster  : SM11 //////
class JB_63 : public Virtual_Input{
    public : 
        JB_63(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Gold Cliff : SM12 //////
class MV_64 : public Virtual_Input{
    public : 
        MV_64(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Zelda (double screen) : SM12 //////
class ZL_65 : public Virtual_Input{
    public : 
        ZL_65(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_RIGHT: cpu->input_set(0, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// DonkeyKong Jr (Wide Screen) : SM10 //////
class DJ_101 : public Virtual_Input{
    public : 
        DJ_101(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Manhole (Wide Screen) : SM10 //////
class NH_103 : public Virtual_Input{
    public : 
        NH_103(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 2, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break; // 
                        default: break; } break;
                default: break;
            }
        }
};



////// Donkey Kong Jr (panorama) : SM11 //////
class CJ_93 : public Virtual_Input{
    public : 
        CJ_93(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(2, 2, state); break;
                        case BUTTON_GAMEB: cpu->input_set(2, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(1, 0, state); break;
                        case BUTTON_UP: cpu->input_set(1, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 2, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Super Mario Bros : SM11 //////
class YM_801 : public Virtual_Input{
    public : 
        YM_801(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Ice Climber : SM11 //////
class DR_802 : public Virtual_Input{
    public : 
        DR_802(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(3, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Balloon Fight : SM11 //////
class BF_803 : public Virtual_Input{
    public : 
        BF_803(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(0, 1, state); break;
                        case BUTTON_TIME: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(1, 0, state); break;
                        case BUTTON_RIGHT: cpu->input_set(1, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(1, 2, state); break;
                        case BUTTON_LEFT: cpu->input_set(1, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(2, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario's Cement Factory (Table Top) : SM11 //////
class CM_72 : public Virtual_Input{
    public : 
        CM_72(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Snoopy (Table Top) : SM11 //////
class SM_91 : public Virtual_Input{
    public : 
        SM_91(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario's Bombs Away (Table Top) : SM11 //////
class TB_94 : public Virtual_Input{
    public : 
        TB_94(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};



////// Popeye (Table Top) : SM11 //////
class PG_92 : public Virtual_Input{
    public : 
        PG_92(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Donkey Kong Circus / Mickey Mouse (panorama)  : SM11 //////
class DC_95 : public Virtual_Input{
    public : 
        DC_95(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario's Cement Factory (Widescreen) : SM10 //////
class ML_102 : public Virtual_Input{
    public : 
        ML_102(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Tropical Fish : SM10 //////
class TF_104 : public Virtual_Input{
    public : 
        TF_104(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 1, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Mario The Juggler  : SM11 //////
class MB_108 : public Virtual_Input{
    public : 
        MB_108(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Spitball Sparky : SM11 //////
class BU_201 : public Virtual_Input{
    public : 
        BU_201(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 1, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                default: break;
            }
        }
};


////// Crab Grab : SM11 //////
class UD_202 : public Virtual_Input{
    public : 
        UD_202(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_LEFTRIGHT;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_RIGHT: cpu->input_set(0, 0, state); break;
                        case BUTTON_LEFT: cpu->input_set(0, 2, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 1, state); break;
                        case BUTTON_DOWN: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Shuttle Voyage : SM11 //////
class MG_8 : public Virtual_Input{
    public : 
        MG_8(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(7, 0, state); break; // Mode
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(6, 0, state); break; // 
                        case BUTTON_DOWN: cpu->input_set(6, 1, state); break; // 
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(6, 3, state); break;
                        default: break; } break;
                default: break;
            }
        }
};

////// Diver's Adventure : SM11 //////
class DA_37 : public Virtual_Input{
    public : 
        DA_37(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_2_BUTTON_UPDOWN;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(0, 0, state); break; 
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break; 
                        default: break; } break;
                default: break;
            }
        }
};


////// Clever Chicken (Tronica) : SM11 //////
class CC_38V : public Virtual_Input{
    public : 
        CC_38V(SM5XX* c) : Virtual_Input(c) {
            // Based on MAME hh_sm510.cpp (trclchick): IN.0 uses Right/Left/Down (no Up).
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_4_BUTTON_DIRECTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 0, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: down.set_primary(cpu, 0, 3, state); break;  // D-pad Up -> DOWN input
                        case BUTTON_DOWN: cpu->input_set(0, 1, state); break;        // LEFT input
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_UP: down.set_secondary(cpu, 0, 3, state); break; // X -> DOWN input
                        case BUTTON_DOWN: cpu->input_set(0, 0, state); break;      // RIGHT input
                        default: break; } break;
                default: break;
            }
        }

    private:
        OrKBit down;
};

////// Space Rescue / Thunder Ball MG_9/FR_23 : SM11 //////
class MG_9 : public Virtual_Input{
    public : 
        MG_9(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_1_BUTTON_ACTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_TIME: cpu->input_set(1, 3, state); break;
                        case BUTTON_GAMEB: cpu->input_set(1, 1, state); break;                        
                        case BUTTON_GAMEA: cpu->input_set(1, 2, state); break;
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 3, state); break;
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(0, 0, state); break;
                        default: break; } break;
                default: break;            }
        }
};


////// punch_out : SM11 //////
class BX_301 : public Virtual_Input{
    public : 
        BX_301(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_4_BUTTON_DIRECTION;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (player) {
                case 1:
                    switch (part) {
                        case PART_SETUP:
                            switch (button) {
                                case BUTTON_TIME: cpu->input_set(6, 0, state); break;
                                case BUTTON_GAMEB: cpu->input_set(6, 1, state); break;
                                case BUTTON_GAMEA: cpu->input_set(6, 2, state); break;
                                default: break; } break;
                        case PART_LEFT:
                            switch (button) {
                                case BUTTON_RIGHT: cpu->input_set(5, 0, state); break;
                                case BUTTON_LEFT: cpu->input_set(5, 1, state); break;
                                case BUTTON_UP: cpu->input_set(3, 1, state); break;
                                case BUTTON_DOWN: cpu->input_set(3, 0, state); break;
                                default: break; } break;
                        case PART_RIGHT:
                            switch (button) {
                                case BUTTON_ACTION: cpu->input_set(1, 0, state); break;
                                default: break; } break;
                        default: break;
                    }
                    break;
                case 2:
                    switch (part) {
                        case PART_LEFT:
                            switch (button) {
                                case BUTTON_RIGHT: cpu->input_set(4, 2, state); break;
                                case BUTTON_LEFT: cpu->input_set(4, 3, state); break;
                                case BUTTON_UP: cpu->input_set(2, 3, state); break;
                                case BUTTON_DOWN: cpu->input_set(2, 2, state); break;
                                default: break; } break;
                        case PART_RIGHT:
                            switch (button) {
                                case BUTTON_ACTION: cpu->input_set(0, 2, state); break;
                                default: break; } break;
                        default: break;
                    }
                    break;
            }
        }
};

/////// Space Adventure SA_12 : SM11 //////
class SA_12 : public Virtual_Input{
    public : 
        SA_12(SM5XX* c) : Virtual_Input(c) {
            left_configuration = CONF_2_BUTTON_UPDOWN;
            right_configuration = CONF_1_BUTTON_ACTION;
        }

        void set_input(uint8_t part, uint8_t button, bool state, uint8_t player = 1) override{
            switch (part) {
                case PART_SETUP:
                    switch (button) {
                        case BUTTON_GAMEA: cpu->input_set(7, 0, state); break; // Mode
                        default: break; } break;
                case PART_LEFT:
                    switch (button) {
                        case BUTTON_UP: cpu->input_set(5, 2, state); break; // JOYSTICK_RIGHT
                        case BUTTON_DOWN: cpu->input_set(5, 0, state); break; // Left/Sound
                        default: break; } break;
                case PART_RIGHT:
                    switch (button) {
                        case BUTTON_ACTION: cpu->input_set(5, 1, state); break; // Fire
                        default: break; } break;
                default: break;
            }
        }
};





//////////////////// Get good Game & Watch input Configuration ////////////////////

inline Virtual_Input* get_input_config(SM5XX* cpu, std::string ref_game){
    /* SM5A */
    if (ref_game == "AC_01") { return new AC_01(cpu); } // BALL
    else if (ref_game == "FL_02") { return new FL_02(cpu); } // Flagman
    else if (ref_game == "MT_03") { return new MT_03(cpu); } // Vermin
    else if (ref_game == "RC_04") { return new RC_04(cpu); } // Fire
    else if (ref_game == "IP_05" || ref_game == "IP_15") { return new IP_05(cpu); } // Judge
    else if (ref_game == "MH_06") { return new MH_06(cpu); } // Manhole
    else if (ref_game == "CN_07" || ref_game == "CN_17") { return new CN_07(cpu); } // Helmet    
    else if (ref_game == "LN_08") { return new LN_08(cpu); } // Lion
    else if (ref_game == "PR_21") { return new PR_21(cpu); } // Parachute
    else if (ref_game == "OC_22" || ref_game == "IM_03") { return new OC_22(cpu); } // Octopus / Tayny okeana
    else if (ref_game == "PP_23") { return new PP_23(cpu); } // Popeye
    else if (ref_game == "FP_24" || ref_game == "IM_04") { return new FP_24(cpu); } // Chef / Vesyolyy povar
    else if (ref_game == "MC_25" || ref_game == "EG_26" || ref_game == "IM_53") { return new MC_25(cpu); } // Mickey Mouse / Egg / Ataka asteroidov
    else if (ref_game == "IM_19" || ref_game == "ECIRCUS" || ref_game == "IM_10") { return new MC_25(cpu); } // Biathlon / Circus / Hockey
    else if (ref_game == "IM_50" || ref_game == "IM_32" || ref_game == "IM_33") { return new MC_25(cpu); } // Kosmicheskiy polyot / Kot-rybolov / Kvaka-zadavaka
    else if (ref_game == "IM_51" || ref_game == "IM_49" || ref_game == "IM_02") { return new MC_25(cpu); } // Morskaja ataka / Nochnye vorishki / Nu, pogodi!
    else if (ref_game == "IM_16" || ref_game == "IM_13" || ref_game == "IM_22") { return new MC_25(cpu); } // Okhota / Razvedchiki kosmosa / Vesyolye futbolisty
    else if (ref_game == "FR_27" || ref_game == "IM_09") { return new FR_27(cpu); } // Fire (Wide Screen) / Kosmicheskiy most
    else if (ref_game == "SM_11" || ref_game == "SG_21") { return new SM_11(cpu); } // Space Mission / Spider (Tronica)
    else if (ref_game == "SK_10") { return new SK_10(cpu); } // Super Goal Keeper (Tronica)
    else if (ref_game == "IM_23") { return new IM_23(cpu); } // Autoslalom (Elektronika)

    /* SM510 */
    else if (ref_game == "TL_28") { return new TL_28(cpu); } // Turtle Bridge
    else if (ref_game == "ID_29") { return new ID_29(cpu); } // Fire Attack
    else if (ref_game == "SP_30") { return new SP_30(cpu); } // Snoopy Tennis
    else if (ref_game == "OP_51") { return new OP_51(cpu); } // Oil Panic
    else if (ref_game == "DK_52") { return new DK_52(cpu); } // Donkey kong
    else if (ref_game == "DM_53") { return new DM_53(cpu); } // Mickey & Donald 
    else if (ref_game == "GH_54") { return new GH_54(cpu); } // Green House 
    else if (ref_game == "JR_55") { return new JR_55(cpu); } // Donkey Kong II 
    else if (ref_game == "MW_56") { return new MW_56(cpu); } // Mario Bros 
    else if (ref_game == "LP_57") { return new LP_57(cpu); } // Rain Shower 
    else if (ref_game == "TC_58") { return new TC_58(cpu); } // Life Boat 
    else if (ref_game == "MG_61") { return new MG_61(cpu); } // Squish 
    else if (ref_game == "DJ_101") { return new DJ_101(cpu); } // DK JR (Wide Screen)
    else if (ref_game == "ML_102") { return new ML_102(cpu); } // Mario's Cement Factory (Wide Screen)
    else if (ref_game == "NH_103") { return new NH_103(cpu); } // Manhole (Wide Screen)
    else if (ref_game == "TF_104") { return new TF_104(cpu); } // Tropical Fish
    else if (ref_game == "BU_201") { return new BU_201(cpu); } // Spitball Sparky
    else if (ref_game == "UD_202") { return new UD_202(cpu); } // Crab Grab
    else if (ref_game == "MG_8" || ref_game == "TG_18")   { return new MG_8(cpu); }   // Shuttle Voyage / Thief in Garden (Tronica)
    else if (ref_game == "DA_37")   { return new DA_37(cpu); }   // Diver's Adventure (Tronica)
    else if (ref_game == "CC_38V")  { return new CC_38V(cpu); }  // Clever Chicken (Tronica)
    else if (ref_game == "MG_9" || ref_game == "FR_23") { return new MG_9(cpu); } // Space Rescue / Thunder Ball (Tronica)
    

    /* SM511/SM512 */
    else if (ref_game == "PB_59") { return new PB_59(cpu); } // Pinball
    else if (ref_game == "BJ_60") { return new BJ_60(cpu); } // Black jack
    else if (ref_game == "BD_62") { return new BD_62(cpu); } // Bomb Sweeper
    else if (ref_game == "JB_63") { return new JB_63(cpu); } // Safe Buster
    else if (ref_game == "MV_64") { return new MV_64(cpu); } // Gold Cliff
    else if (ref_game == "ZL_65") { return new ZL_65(cpu); } // Zelda    
    else if (ref_game == "CM_72" || ref_game == "CM_72A") { return new CM_72(cpu); } // Mario's Cement Factory
    else if (ref_game == "SM_91") { return new SM_91(cpu); } // Snoopy (table top)
    else if (ref_game == "PG_92") { return new PG_92(cpu); } // Popeye (table top)
    else if (ref_game == "CJ_93" || ref_game == "IM_12") { return new CJ_93(cpu); } // DK JR (Panorama) / Vinni-Pukh
    else if (ref_game == "TB_94") { return new TB_94(cpu); } // Mario's Bombs Away
    else if (ref_game == "DC_95" || ref_game == "MK_96") { return new DC_95(cpu); } // Donkey Kong Circus / Mickey Mouse
    else if (ref_game == "YM_801" || ref_game == "YM_105") { return new YM_801(cpu); } // Super Mario Bros
    else if (ref_game == "DR_802" || ref_game == "DR_106") { return new DR_802(cpu); } // Climber
    else if (ref_game == "BF_803" || ref_game == "BF_107") { return new BF_803(cpu); } // Balloon Fight
    else if (ref_game == "MB_108") { return new MB_108(cpu); } // Mario The Juggler
    else if (ref_game == "BX_301" || ref_game == "AK_302" || ref_game == "HK_303") { return new BX_301(cpu); } // all vs g&w (3)
    else if (ref_game == "SA_12") { return new SA_12(cpu); } // Space Adventure (Tronica)
    
    return nullptr;
}
