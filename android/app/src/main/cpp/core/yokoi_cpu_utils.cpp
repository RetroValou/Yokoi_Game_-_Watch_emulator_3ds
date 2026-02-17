#include "yokoi_cpu_utils.h"

#include <time.h>

#include "SM5XX/SM5XX.h"
#include "SM5XX/SM5A/SM5A.h"
#include "SM5XX/SM510/SM510.h"
#include "SM5XX/SM511_SM512/SM511_2.h"
#include "David_and_John/David_And_John_fake_cpu.h"


void yokoi_cpu_set_time_if_needed(SM5XX* cpu) {
    if (!cpu || cpu->is_time_set()) {
        return;
    }

    time_t now = time(nullptr);
    tm* t = localtime(&now);
    if (!t) {
        return;
    }

    cpu->set_time((uint8_t)t->tm_hour, (uint8_t)t->tm_min, (uint8_t)t->tm_sec);
    cpu->time_set(true);
}

bool yokoi_cpu_create_instance(std::unique_ptr<SM5XX>& out, const uint8_t* rom, uint16_t size_rom) {
    if (!rom || size_rom == 0) {
        return false;
    }

    if(size_rom == 4){ //No CPU, personalyse game
        if(rom[0] == 0xFF && rom[1] == 0xFF && rom[2] == 0xFF){ // David and John
            out = std::make_unique<David_And_John_fake_cpu>();
            return true;
        }
    }
    if (size_rom == 1856) {
        out = std::make_unique<SM5A>();
        return true;
    }
    if (size_rom == 4096) {
        // Heuristic from 3DS main.cpp
        for (int i = 0; i < 16; i++) {
            if (rom[i + 704] != 0x00) { // Rom from sm510 has 63 word + 1 empty word per line/pages, sm511/12 has 64 word
                out = std::make_unique<SM511_2>();
                return true;
            }
        }
        out = std::make_unique<SM510>();
        return true;
    }
    return false;
}
