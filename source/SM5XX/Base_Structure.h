#pragma once


struct ProgramCounter {
    uint8_t col;
    uint8_t line;
    uint8_t word;
};

struct RAMAddress {
    uint8_t col;
    uint8_t line;
};

inline int16_t min(int16_t a, int16_t b){ if(a>b){ return b; } return a;}

inline int16_t max(int16_t a, int16_t b){ if(a>b){ return a; } return b;}

