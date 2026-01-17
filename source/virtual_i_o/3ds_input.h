#include <3ds.h>
#include <cstdint>
#include "virtual_i_o/virtual_input.h"
#include "std/timer.h"


constexpr uint64_t PROTECT_BAD_INPUT = 5* 1000ULL; // in us
constexpr uint64_t INCREASE_HELD = 40* 1000ULL; // in us



class Input_Manager_3ds {

    public:

    private:
        u32 last_input = 0;
        uint64_t last_time_input = 0;

        u32 kDown;
        u32 kHeld;
        u32 kUp;

    public: 
        void input_Update();
        bool input_justPressed(u32 input, bool protect_bad = true);
        bool input_isHeld(u32 input);
        bool input_Held_Increase(u32 input, uint64_t time_between = INCREASE_HELD);
        void input_GW_Update(Virtual_Input* v_input);

};
