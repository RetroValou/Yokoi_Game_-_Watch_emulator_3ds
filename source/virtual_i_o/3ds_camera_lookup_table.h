#pragma once
#include <cstdint>

constexpr uint8_t TILE_ORDER_X[64]= {0,0,2,2,0,0,2,2,4,4,6,6,4,4,6,6,0,0,2,2,0,0,2,2,4,4,6,6,4,4,6,6};
constexpr uint8_t TILE_ORDER_Y[64]= {0,1,0,1,2,3,2,3,0,1,0,1,2,3,2,3,4,5,4,5,6,7,6,7,4,5,4,5,6,7,6,7};