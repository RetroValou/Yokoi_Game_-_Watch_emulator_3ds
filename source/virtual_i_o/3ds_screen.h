#pragma once
#include <vector>
#include <cstdint>
#include "SM5XX/SM5XX.h"
#include "std/segment.h"

#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>


#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

constexpr uint16_t nb_segments_max = 512;
constexpr uint16_t nb_text_max = 1024;
constexpr uint16_t nb_img_interface_max = 10;

constexpr uint32_t SEGMENT_COLOR[5] {0x080908, /* classic black segment */
                        0x9992e7, 0x58b9a0, 0xff677c, 0x3db8e4 }; /* Color of 2 games watch color segment*/
#define CLEAR_COLOR 0xdbe2bb
#define FOND_COLOR_MENU 0xe7eff6


typedef struct { float position[3]; float texcoord[2]; } vertex;
typedef struct { uint16_t indice; uint8_t screen; } letter;



class Virtual_Screen {

    public:
        std::vector<uint16_t> size_screen; // all size screen. Size or Screen i -> x_size = size_screen[i*2] / y_size = size_screen[i*2+1]
        uint8_t nb_screen = 0;
        std::vector<uint8_t> parameter_screen;
        std::vector<bool> mask_screen;

    private: 
        bool already_load_game = false;
        C3D_Tex texture_game;
        C3D_Tex background;
        C3D_Tex text_texture;
        C3D_Tex img_texture[nb_img_interface_max];

        std::vector<Segment> list_segment;
        uint32_t index_segment_screen[4];
        const uint16_t* segment_info;

        bool is_mask = false;
        bool double_in_one_screen = false;

        const uint16_t* background_info;
        bool img_background = false;
        std::vector<uint16_t> background_ind_vertex;

        uint32_t curr_fond_color;
        uint32_t curr_alpha_color;

        vertex* vertex_data;
        DVLB_s* vshader_dvlb;
        shaderProgram_s program;
        int uLoc_projection, uLoc_modelView;
        C3D_Mtx projection_up, projection_down;
        C3D_Mtx modelView_up, modelView_down;

        C3D_RenderTarget* target_up;
        C3D_RenderTarget* target_down;

        uint32_t index_start_texte;
        uint32_t size_text_screen_0;
        uint32_t size_text_screen_1;
        std::vector<uint16_t> indice_img;

    public:
        void config_screen();
        void load_visual(std::string path_segment
                        , const Segment* segment_list, const size_t size_segment_list
                        , const uint16_t* v_segment_info
                        , std::string path_background 
                        , const uint16_t* v_background_info );
        bool init_visual();
        bool update_buffer_video(SM5XX* cpu);
        void update_screen();
        void Quit_Game();
        void Exit();
        void update_text(bool clean = true);
        void update_img(bool clean = true);
        void set_text(const std::string& txt, int16_t x_pos_init, int16_t y_pos, uint8_t screen = 0, uint8_t multiply_size = 1);
        void delete_all_text();
        void delete_all_img();
        void set_img(const std::string& path, const uint16_t* info
                        , int16_t x_pos, int16_t y_pos, uint8_t img_i);

    private:
        void protect_blinking(Segment *seg, bool new_state);
        void set_base_environnement();
        void set_alpha_environnement(uint8_t alpha_multiply = 0xFF);
        void set_color_environnement(uint32_t color);
        void change_alpha_color_environnement(uint32_t color_, uint8_t alpha_multiply = 0xFF);
        void set_screen_up();
        void set_screen_down();
        void send_vbo();
};
