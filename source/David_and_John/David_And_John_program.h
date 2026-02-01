#pragma once
//#include <vector> declared in SM5xx.h
#include <cstdint>
#include <string>

class David_And_John_fake_cpu;


class David_And_John_program {

    public:
        explicit David_And_John_program(David_And_John_fake_cpu* v_fake_cpu) :
            fake_cpu(v_fake_cpu)
        {}

        virtual bool get_segments_state(uint8_t line, uint8_t word);
        virtual void execute_program_loop();
        virtual void init_program();

    protected:
        David_And_John_fake_cpu* fake_cpu;
};
 