#pragma once
#include <cstdint>

// ===============================
// Useful indices for ROM info
// ===============================

// Valid for background, segment info and console info
constexpr int I_TEX_W = 0;
constexpr int I_TEX_H = 1;



// ===============================
// Background = bg
// ===============================

static inline int bg_jump(int i_screen) { return 2 + i_screen * 4; }
static inline int i_bg_shadow(int nb_screen) { return bg_jump(nb_screen); }
static inline int i_bg_in_front(int nb_screen) { return bg_jump(nb_screen)+1; }
static inline int i_camera(int nb_screen) { return bg_jump(nb_screen)+2; }

static inline int i_bg_x(int i_screen) { return bg_jump(i_screen) + 0; }
static inline int i_bg_y(int i_screen) { return bg_jump(i_screen) + 1; }
static inline int i_bg_w(int i_screen) { return bg_jump(i_screen) + 2; }
static inline int i_bg_h(int i_screen) { return bg_jump(i_screen) + 3; }



// ===============================
// Image (console illustration)
// ===============================

constexpr int I_IMG_X = 2;
constexpr int I_IMG_Y = 3;
constexpr int I_IMG_W = 4;
constexpr int I_IMG_H = 5;



// ===============================
// Segments info
// ===============================
constexpr int I_SEG_MULTI = 2;
constexpr int I_SEG_MASK = 3;

static inline int i_seg_scr_w(int i_screen) { return 4 + 2*i_screen; }
static inline int i_seg_scr_h(int i_screen) { return 5 + 2*i_screen; }


