
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
	{ { 0,1,0 }, { 43,49 }, { 67,976 }, { 60,47 }, 0, 0, false, false, 0 }, { { 0,1,1 }, { 51,50 }, { 1,383 }, { 58,46 }, 0, 0, false, false, 0 }, { { 0,1,10 }, { 35,49 }, { 1,821 }, { 68,47 }, 0, 0, false, false, 0 }, { { 0,1,11 }, { 51,49 }, { 1,724 }, { 66,47 }, 0, 0, false, false, 0 }, { { 0,1,12 }, { 34,117 }, { 1,773 }, { 69,46 }, 0, 0, false, false, 0 }, { { 0,1,13 }, { 51,116 }, { 1,625 }, { 65,47 }, 0, 0, false, false, 0 }, { { 0,1,14 }, { 35,182 }, { 1,920 }, { 68,48 }, 0, 0, false, false, 0 }, { { 0,1,15 }, { 51,184 }, { 1,528 }, { 66,46 }, 0, 0, false, false, 0 }, { { 0,1,2 }, { 42,117 }, { 63,480 }, { 61,46 }, 0, 0, false, false, 0 }, { { 0,1,3 }, { 51,116 }, { 69,724 }, { 58,47 }, 0, 0, false, false, 0 }, { { 0,1,4 }, { 45,182 }, { 69,572 }, { 58,48 }, 0, 0, false, false, 0 }, { { 0,1,5 }, { 51,184 }, { 1,431 }, { 60,46 }, 0, 0, false, false, 0 }, { { 0,2,0 }, { 16,78 }, { 69,322 }, { 22,17 }, 0, 0, false, false, 0 }, { { 0,2,1 }, { 118,65 }, { 70,888 }, { 36,30 }, 0, 0, false, false, 0 }, { { 0,2,2 }, { 16,145 }, { 93,322 }, { 22,17 }, 0, 0, false, false, 0 }, { { 0,2,3 }, { 118,132 }, { 72,856 }, { 36,30 }, 0, 0, false, false, 0 }, { { 0,2,4 }, { 16,212 }, { 18,295 }, { 22,16 }, 0, 0, false, false, 0 }, { { 0,2,5 }, { 118,198 }, { 71,824 }, { 36,30 }, 0, 0, false, false, 0 }, { { 0,3,0 }, { 0,69 }, { 18,313 }, { 15,26 }, 0, 0, false, false, 0 }, { { 0,3,1 }, { 152,66 }, { 109,825 }, { 18,29 }, 0, 0, false, false, 0 }, { { 0,3,2 }, { 0,134 }, { 110,893 }, { 15,28 }, 0, 0, false, false, 0 }, { { 0,3,3 }, { 155,133 }, { 111,945 }, { 15,29 }, 0, 0, false, false, 0 }, { { 0,3,4 }, { 0,201 }, { 1,312 }, { 15,27 }, 0, 0, false, false, 0 }, { { 0,3,5 }, { 155,202 }, { 35,313 }, { 15,26 }, 0, 0, false, false, 0 }, { { 0,4,0 }, { 0,96 }, { 69,528 }, { 44,10 }, 0, 0, false, false, 0 }, { { 0,4,1 }, { 106,96 }, { 61,389 }, { 64,10 }, 0, 0, false, false, 0 }, { { 0,4,2 }, { 0,162 }, { 72,783 }, { 44,10 }, 0, 0, false, false, 0 }, { { 0,4,3 }, { 106,162 }, { 61,377 }, { 64,10 }, 0, 0, false, false, 0 }, { { 0,4,4 }, { 0,229 }, { 67,365 }, { 44,10 }, 0, 0, false, false, 0 }, { { 0,4,5 }, { 106,229 }, { 1,365 }, { 64,10 }, 0, 0, false, false, 0 }, { { 0,5,0 }, { 51,21 }, { 72,795 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,5,1 }, { 27,21 }, { 61,401 }, { 27,27 }, 0, 0, false, false, 0 }, { { 0,5,2 }, { 4,21 }, { 101,795 }, { 26,27 }, 0, 0, false, false, 0 }, { { 0,6,0 }, { 43,5 }, { 118,782 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,1 }, { 43,5 }, { 12,256 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,6,2 }, { 43,5 }, { 115,529 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,3 }, { 43,5 }, { 117,325 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,4 }, { 43,5 }, { 116,932 }, { 11,11 }, 0, 0, false, false, 0 }, { { 0,6,5 }, { 43,5 }, { 117,312 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,6 }, { 43,5 }, { 117,299 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,7 }, { 43,5 }, { 1,299 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,8 }, { 43,5 }, { 1,286 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,6,9 }, { 43,5 }, { 12,282 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,0 }, { 33,5 }, { 1,273 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,1 }, { 33,5 }, { 85,264 }, { 7,11 }, 0, 0, false, false, 0 }, { { 0,7,2 }, { 33,5 }, { 23,282 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,3 }, { 33,5 }, { 60,290 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,4 }, { 33,5 }, { 117,364 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,7,5 }, { 33,5 }, { 71,290 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,6 }, { 33,5 }, { 82,290 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,7 }, { 33,5 }, { 93,290 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,8 }, { 33,5 }, { 104,290 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,7,9 }, { 33,5 }, { 115,286 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,0 }, { 22,5 }, { 34,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,1 }, { 22,5 }, { 1,247 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,8,2 }, { 22,5 }, { 12,269 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,3 }, { 22,5 }, { 23,269 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,4 }, { 22,5 }, { 117,351 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,8,5 }, { 22,5 }, { 1,260 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,6 }, { 22,5 }, { 45,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,7 }, { 22,5 }, { 56,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,8 }, { 22,5 }, { 67,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,8,9 }, { 22,5 }, { 78,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,0 }, { 11,5 }, { 89,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,1 }, { 11,5 }, { 22,256 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,2 }, { 11,5 }, { 100,277 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,3 }, { 12,5 }, { 45,264 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,4 }, { 12,5 }, { 117,338 }, { 10,11 }, 0, 0, false, false, 0 }, { { 0,9,5 }, { 12,5 }, { 55,264 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,6 }, { 12,5 }, { 65,264 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,7 }, { 12,5 }, { 75,264 }, { 8,11 }, 0, 0, false, false, 0 }, { { 0,9,8 }, { 11,5 }, { 111,273 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,9,9 }, { 11,5 }, { 34,264 }, { 9,11 }, 0, 0, false, false, 0 }, { { 0,0,0 }, { 36,138 }, { 90,413 }, { 29,15 }, 0, 1, false, false, 0 }, { { 0,0,1 }, { 83,134 }, { 71,923 }, { 43,19 }, 0, 1, false, false, 0 }, { { 0,1,16 }, { 37,9 }, { 1,576 }, { 65,47 }, 0, 1, false, false, 0 }, { { 0,1,17 }, { 51,8 }, { 1,674 }, { 64,48 }, 0, 1, false, false, 0 }, { { 0,1,18 }, { 35,75 }, { 1,870 }, { 67,48 }, 0, 1, false, false, 0 }, { { 0,1,19 }, { 51,70 }, { 1,970 }, { 64,53 }, 0, 1, false, false, 0 }, { { 0,1,6 }, { 42,9 }, { 1,479 }, { 60,47 }, 0, 1, false, false, 0 }, { { 0,1,7 }, { 51,8 }, { 63,430 }, { 58,48 }, 0, 1, false, false, 0 }, { { 0,1,8 }, { 43,75 }, { 68,622 }, { 59,48 }, 0, 1, false, false, 0 }, { { 0,1,9 }, { 51,73 }, { 68,672 }, { 59,50 }, 0, 1, false, false, 0 }, { { 0,2,6 }, { 15,38 }, { 69,303 }, { 22,17 }, 0, 1, false, false, 0 }, { { 0,2,7 }, { 117,25 }, { 69,540 }, { 37,30 }, 0, 1, false, false, 0 }, { { 0,2,8 }, { 15,104 }, { 93,303 }, { 22,17 }, 0, 1, false, false, 0 }, { { 0,2,9 }, { 117,91 }, { 72,944 }, { 37,30 }, 0, 1, false, false, 0 }, { { 0,3,6 }, { 0,27 }, { 110,863 }, { 15,28 }, 0, 1, false, false, 0 }, { { 0,3,7 }, { 153,34 }, { 42,290 }, { 16,21 }, 0, 1, false, false, 0 }, { { 0,3,8 }, { 0,95 }, { 52,313 }, { 15,26 }, 0, 1, false, false, 0 }, { { 0,3,9 }, { 154,89 }, { 108,542 }, { 15,28 }, 0, 1, false, false, 0 }, { { 0,4,6 }, { 0,55 }, { 67,353 }, { 44,10 }, 0, 1, false, false, 0 }, { { 0,4,7 }, { 105,55 }, { 1,353 }, { 64,10 }, 0, 1, false, false, 0 }, { { 0,4,8 }, { 0,122 }, { 67,341 }, { 44,10 }, 0, 1, false, false, 0 }, { { 0,4,9 }, { 105,122 }, { 1,341 }, { 64,10 }, 0, 1, false, false, 0 }
};  const size_t size_segment_GW_d_and_j_fabric = sizeof(segment_GW_d_and_j_fabric)/sizeof(segment_GW_d_and_j_fabric[0]); 
 const uint16_t segment_info_d_and_j_fabric[] = { 128, 1024, 1, 0, 170, 240, 169, 154}; 

const std::string path_background_d_and_j_fabric = "";
const uint16_t background_info_d_and_j_fabric[] = { 256, 512, 0, 0, 170, 240, 0, 0, 169, 154, 1, 0, 0 }; 



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

