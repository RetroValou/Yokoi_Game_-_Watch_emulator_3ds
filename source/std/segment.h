#pragma once

#include <cstdint>

struct Segment {
    uint8_t id[3]; // col, line, word
    int pos_scr[2]; // position screen : x, y
    uint16_t pos_tex[2]; // position in texture : x, y 
    uint16_t size_tex[2]; // size in texture : x, y
    uint8_t color_index; // for only 2 game watch game "color", 0 = default color
    uint8_t screen; // screen 0 or 1
    bool state;
    bool buffer_state;
    uint16_t index_vertex;
};
