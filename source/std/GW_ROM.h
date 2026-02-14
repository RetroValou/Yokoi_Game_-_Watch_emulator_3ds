
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "segment.h"

struct GW_rom {
    static constexpr uint8_t MANUFACTURER_NINTENDO = 0;
    static constexpr uint8_t MANUFACTURER_TRONICA = 1;
    static constexpr uint8_t MANUFACTURER_ELEKTRONIKA = 2;
    static constexpr uint8_t MANUFACTURER_TIGER = 3;
    static constexpr uint8_t MANUFACTURER_DAVID_AND_JOHN = 4;
    static constexpr uint8_t MANUFACTURER_COUNT = 5;

    const std::string name;
    const std::string ref;
    const std::string date;

    // Manufacturer id used by the menu to group navigation (no extra rompack metadata).
    const uint8_t manufacturer;

    const uint8_t* rom; const size_t size_rom;
    const uint8_t* melody; const size_t size_melody;

    const std::string path_segment;
    const Segment* segment; const size_t size_segment;
    const uint16_t* segment_info; 

    const std::string path_background;
    const uint16_t* background_info; 

    const std::string path_console;
    const uint16_t* console_info; 

    GW_rom(const std::string& n, const std::string& r, const std::string& d,
        const uint8_t* rom_, size_t rom_size,
        const uint8_t* melody_, size_t melody_size,
        const std::string& path_seg,
        const Segment* seg_, size_t seg_size,
        const uint16_t* seg_info,
        const std::string& path_bg,
        const uint16_t* bg_info,
        const std::string& path_cs,
        const uint16_t* cs_info,
        uint8_t manufacturer_ = MANUFACTURER_NINTENDO
        )
            : name(n), ref(r), date(d),
            manufacturer(manufacturer_),
            rom(rom_), size_rom(rom_size),
            melody(melody_), size_melody(melody_size),
            path_segment(path_seg),
            segment(seg_), size_segment(seg_size),
            segment_info(seg_info),
            path_background(path_bg),
            background_info(bg_info),
            path_console(path_cs),
            console_info(cs_info)
            {}
};
