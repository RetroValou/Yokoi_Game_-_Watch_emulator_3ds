#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include "SM5XX/SM5XX.h"
#include "std/segment.h"
#include "std/settings.h"

#include "virtual_i_o/3ds_camera.h"


#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>


#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

constexpr uint16_t nb_segments_max = 512;
constexpr uint16_t nb_text_max = 2048;
constexpr uint16_t nb_img_interface_max = 10;

constexpr uint32_t SEGMENT_COLOR[5] {0x080908, /* classic black segment */
                        0x9992e7, 0x58b9a0, 0xff677c, 0x3db8e4 }; /* Color of 2 games watch color segment*/
// CLEAR_COLOR is now configurable via g_settings.background_color
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
        C3D_Tex noise_texture;
        C3D_Tex img_texture[nb_img_interface_max];
        std::string img_texture_path[nb_img_interface_max];

        std::vector<Segment> list_segment;
        uint32_t index_segment_screen[4];
        const uint16_t* segment_info;

        bool is_mask = false;
        bool double_in_one_screen = false;

        const uint16_t* background_info;
        bool img_background = false;
        std::vector<uint16_t> background_ind_vertex;
        bool need_redraw_bottom = false; // Flag to redraw bottom screen after settings

        uint32_t curr_fond_color;
        uint32_t curr_fond_color_noise;
        uint32_t curr_alpha_color;

        vertex* vertex_data;
        DVLB_s* vshader_dvlb;
        shaderProgram_s program;
        int uLoc_projection, uLoc_modelView;
        C3D_Mtx projection_up, projection_down;
        C3D_Mtx modelView_up, modelView_down;

        C3D_Mtx* modelView_curr;

        C3D_RenderTarget* target_up;
        C3D_RenderTarget* target_right;
        C3D_RenderTarget* target_down;

        float slider_3d;
        float eye_offset_value;

        float offset_segment;
        float offset_mark_segment;
        float offset_background;
        float offset_fond;

        uint32_t index_start_texte;
        uint32_t size_text_screen_0;
        uint32_t size_text_screen_1;
        std::vector<uint16_t> indice_img;

        Camera_3ds cam;

    public:
        void config_screen();
        bool load_visual(std::string path_segment
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
        void refresh_settings(); // Update visual settings from g_settings
        bool is_double_in_one_screen() const { return double_in_one_screen; }

        std::vector<int> pos_fond;

    private:
        void protect_blinking(Segment *seg, bool new_state);

        void set_base_environnement();
        void set_alpha_environnement(uint8_t alpha_multiply = 0xFF);
        void set_color_environnement(uint32_t color);
        void set_gradient_color(uint32_t color1, uint32_t color2);
        void change_alpha_color_environnement(uint32_t color_, uint8_t alpha_multiply = 0xFF);

        void set_screen_up(bool on_left_eye = true);
        void set_screen_down();
        void send_vbo();

        int set_good_screen(int curr_screen);
  
        void update_slider_3d_value(int nb_render);
        void update_eye_offset_value(int nb_render, int i_render);
        void update_offset_fond();
        void update_offset_background();
        void update_offset_segment();

        float get_eye_offset_segment(int nb_render, int i_render, bool is_active);
        void apply_3d_segment(C3D_Mtx* curr_modelView, int i_render, bool is_active);
        void apply_3d_background(C3D_Mtx* curr_modelView, int i_render);
        void apply_3d_fond(C3D_Mtx* curr_modelView, int i_render, float zoom);

        void create_fond(int nb_render, int i_render, int curr_screen);
        void create_shadow(int nb_render, int i_render, int curr_screen);
        void create_segment(int nb_render, int i_render, int curr_screen);
        void create_background(int nb_render, int i_render, int curr_screen);
        void create_border(int nb_render, int i_render, int curr_screen);

};
