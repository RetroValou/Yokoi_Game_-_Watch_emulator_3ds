
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "d_and_j_fabric.h"


const uint8_t rom_GW_d_and_j_fabric[] = {
    0xFF, 0xFF, 0xFF, 0x04
}; const size_t size_rom_GW_d_and_j_fabric = sizeof(rom_GW_d_and_j_fabric)/sizeof(rom_GW_d_and_j_fabric[0]);

const uint8_t melody_GW_d_and_j_fabric[1] = {0}; 
	const size_t size_melody_GW_d_and_j_fabric = 0;

const std::string path_segment_d_and_j_fabric = "romfs:/gfx/segment_d_and_j_fabric.t3x"; // Visual of segment -> Big unique texture
const Segment segment_GW_d_and_j_fabric[] = {
	{ { 0,0,0 }, { 22,230 }, { 44,960 }, { 18,9 }, 0, 0, false, false, 0 }, { { 0,0,1 }, { 50,227 }, { 1,390 }, { 27,12 }, 0, 0, false, false, 0 }, { { 0,1,0 }, { 26,30 }, { 1,618 }, { 36,29 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 31,31 }, { 1,404 }, { 35,28 }, 0, 0, false, false, 0 }, { { 0,1,10 }, { 21,30 }, { 1,959 }, { 41,29 }, 0, 0, false, false, 0 }, { { 0,1,11 }, { 31,30 }, { 1,866 }, { 40,29 }, 0, 0, false, false, 0 }, { { 0,1,12 }, { 21,71 }, { 1,805 }, { 41,28 }, 0, 0, false, false, 0 }, { { 0,1,13 }, { 31,70 }, { 1,774 }, { 39,29 }, 0, 0, false, false, 0 }, { { 0,1,14 }, { 21,111 }, { 1,928 }, { 41,29 }, 0, 0, false, false, 0 }, { { 0,1,15 }, { 31,112 }, { 1,713 }, { 40,28 }, 0, 0, false, false, 0 }, { { 0,1,16 }, { 22,151 }, { 1,835 }, { 40,29 }, 0, 0, false, false, 0 }, { { 0,1,17 }, { 31,151 }, { 1,743 }, { 39,29 }, 0, 0, false, false, 0 }, { { 0,1,18 }, { 21,192 }, { 1,897 }, { 41,29 }, 0, 0, false, false, 0 }, { { 0,1,19 }, { 31,188 }, { 1,990 }, { 39,33 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 25,71 }, { 1,557 }, { 37,28 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 31,70 }, { 1,526 }, { 35,29 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 27,111 }, { 1,495 }, { 35,29 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 31,112 }, { 1,434 }, { 36,28 }, 0, 0, false, false, 0 }, { { 0,1,6 }, { 25,151 }, { 1,649 }, { 37,29 }, 0, 0, false, false, 0 }, { { 0,1,7 }, { 31,151 }, { 1,464 }, { 35,29 }, 0, 0, false, false, 0 }, { { 0,1,8 }, { 26,192 }, { 1,587 }, { 36,29 }, 0, 0, false, false, 0 }, { { 0,1,9 }, { 31,190 }, { 1,680 }, { 36,31 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 9,47 }, { 44,890 }, { 14,11 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 71,39 }, { 40,692 }, { 23,19 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 9,88 }, { 44,877 }, { 14,11 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 71,80 }, { 40,651 }, { 23,18 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 9,128 }, { 44,864 }, { 14,11 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 71,120 }, { 40,671 }, { 23,19 }, 0, 0, false, false, 0 }, { { 0,2,6 }, { 9,169 }, { 44,851 }, { 14,11 }, 0, 0, false, false, 0 }, { { 0,2,7 }, { 71,161 }, { 40,631 }, { 23,18 }, 0, 0, false, false, 0 }, { { 0,2,8 }, { 9,210 }, { 43,767 }, { 14,10 }, 0, 0, false, false, 0 }, { { 0,2,9 }, { 71,202 }, { 40,611 }, { 23,18 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,42 }, { 44,833 }, { 9,16 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 92,40 }, { 39,591 }, { 11,18 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 0,82 }, { 44,815 }, { 9,16 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 94,81 }, { 44,941 }, { 9,17 }, 0, 0, false, false, 0 }, { { 0,3,4 }, { 0,122 }, { 44,922 }, { 9,17 }, 0, 0, false, false, 0 }, { { 0,3,5 }, { 94,123 }, { 44,797 }, { 9,16 }, 0, 0, false, false, 0 }, { { 0,3,6 }, { 0,163 }, { 44,903 }, { 9,17 }, 0, 0, false, false, 0 }, { { 0,3,7 }, { 93,167 }, { 43,753 }, { 10,12 }, 0, 0, false, false, 0 }, { { 0,3,8 }, { 0,204 }, { 43,779 }, { 9,16 }, 0, 0, false, false, 0 }, { { 0,3,9 }, { 94,200 }, { 52,591 }, { 9,18 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 0,58 }, { 30,340 }, { 27,6 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 64,58 }, { 1,364 }, { 39,6 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 0,99 }, { 30,332 }, { 27,6 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 64,99 }, { 1,356 }, { 39,6 }, 0, 0, false, false, 0 }, { { 0,4,4 }, { 0,139 }, { 30,395 }, { 27,7 }, 0, 0, false, false, 0 }, { { 0,4,5 }, { 64,139 }, { 1,381 }, { 39,7 }, 0, 0, false, false, 0 }, { { 0,4,6 }, { 0,180 }, { 1,331 }, { 27,6 }, 0, 0, false, false, 0 }, { { 0,4,7 }, { 64,180 }, { 1,348 }, { 39,6 }, 0, 0, false, false, 0 }, { { 0,4,8 }, { 0,220 }, { 1,339 }, { 27,7 }, 0, 0, false, false, 0 }, { { 0,4,9 }, { 64,220 }, { 1,372 }, { 39,7 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 31,13 }, { 44,1007 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 17,13 }, { 44,989 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 2,13 }, { 44,971 }, { 16,16 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 26,3 }, { 42,744 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 26,3 }, { 58,740 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 26,3 }, { 55,942 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 26,3 }, { 55,933 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 26,3 }, { 55,924 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 26,3 }, { 55,915 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 26,3 }, { 55,906 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 26,3 }, { 55,842 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 26,3 }, { 55,833 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 26,3 }, { 55,824 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 20,3 }, { 43,713 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 20,3 }, { 59,770 }, { 4,7 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 20,3 }, { 50,713 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 20,3 }, { 51,722 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 20,3 }, { 55,815 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 20,3 }, { 58,722 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 20,3 }, { 57,713 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 20,3 }, { 42,386 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 20,3 }, { 49,386 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 20,3 }, { 56,386 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 13,3 }, { 55,806 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 13,3 }, { 42,377 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 13,3 }, { 55,797 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 13,3 }, { 55,758 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 13,3 }, { 55,951 }, { 7,7 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 13,3 }, { 55,749 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 13,3 }, { 54,788 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 13,3 }, { 54,779 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 13,3 }, { 50,740 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 13,3 }, { 43,731 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 7,3 }, { 49,377 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 7,3 }, { 56,377 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 7,3 }, { 42,368 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 7,3 }, { 49,368 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 7,3 }, { 51,731 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 7,3 }, { 56,368 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 7,3 }, { 43,722 }, { 6,7 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 7,3 }, { 42,359 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 7,3 }, { 42,350 }, { 5,7 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 7,3 }, { 49,359 }, { 5,7 }, 0, 0, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_fabric = sizeof(segment_GW_d_and_j_fabric)/sizeof(segment_GW_d_and_j_fabric[0]); 
 const uint16_t segment_info_d_and_j_fabric[] = { 64, 1024, 1, 0, 103, 240}; 

const std::string path_background_d_and_j_fabric = "";
const uint16_t background_info_d_and_j_fabric[] = { 128, 256, 0, 0, 103, 240, 1, 0, 0 }; 


const std::string path_console_d_and_j_fabric = "romfs:/gfx/console_d_and_j_fabric.t3x";
const uint16_t console_info_d_and_j_fabric[] = { 512, 256, 0, 16, 320, 240}; 




const GW_rom d_and_j_fabric (
    "New Job for John", "D_AND_J_FABRIC", "2024-08-26"
    , rom_GW_d_and_j_fabric, size_rom_GW_d_and_j_fabric
    , melody_GW_d_and_j_fabric, size_melody_GW_d_and_j_fabric
    , path_segment_d_and_j_fabric
    , segment_GW_d_and_j_fabric, size_segment_GW_d_and_j_fabric
    , segment_info_d_and_j_fabric
    , path_background_d_and_j_fabric
    , background_info_d_and_j_fabric
    , path_console_d_and_j_fabric
    , console_info_d_and_j_fabric
    , GW_rom::MANUFACTURER_DAVID_AND_JOHN
);

