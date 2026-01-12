#include <algorithm>
#include <math.h>
#include <string>

#include "virtual_i_o/3ds_screen.h"
#include "std/timer.h"
#include "std/GW_info_reader.h"
#include "std/gw_pack.h"
#include "std/debug_log.h"

#include "vshader_shbin.h" // shader used
#include "virtual_i_o/texte_decoup.h"

constexpr uint16_t _3DS_SCREEN_X[2] = { 400, 320 };
constexpr uint16_t _3DS_SCREEN_Y[2] = { 240, 240 };

constexpr float _LUM_DARK_FOND_NOISE_ = 0.9f; //0.92f; //0.92f;//0.96f;//0.96f;
constexpr float _LUM_LIGHT_FOND_NOISE_ = 1.5f; //1.58f;//1.2f;
constexpr float _ZOOM_SHADOW_ = 0.014f;//0.02f;
constexpr float _COEF_MOVE_SHADOW_ = 0.7f;



////// Useful function  ////////////////////////////////////////////////////////////

static bool loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
{
	Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
	if (!t3x) { return false; }
	Tex3DS_TextureFree(t3x);
	return true;
}

inline u32 RGBtoBGR8(uint32_t rgb) {
    u8 r = (rgb>>16)& 0xFF;
    u8 g = (rgb>>8) & 0xFF;
    u8 b = (rgb)    & 0xFF; 
    return (b << 16) | (g << 8) | r;
}

uint32_t lum_rgb8(uint32_t c, float factor) {
    float r = (c >> 16) & 0xFF;
    float g = (c >> 8)  & 0xFF;
    float b = c & 0xFF;

    r = fminf(fmaxf(r * factor, 0.0f), 255.0f);
    g = fminf(fmaxf(g * factor, 0.0f), 255.0f);
    b = fminf(fmaxf(b * factor, 0.0f), 255.0f);

    return ((int)(r + 0.5f) << 16) |
           ((int)(g + 0.5f) << 8)  |
           ((int)(b + 0.5f));
}


void Virtual_Screen::send_vbo(){
    // send vbo to gpu
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, vertex_data, sizeof(vertex), 2, 0x210);
}


bool loadTexture_file(std::string path, C3D_Tex* tex) {

    // If an external ROM pack is loaded, prefer reading the texture bytes from it.
    // The pack stores files by basename (e.g. "segment_Ball.t3x").
    // Note: the pack generator also uses paths like "romfs:/gfx/console_*.t3x" in game metadata;
    // we still need to resolve those via the pack (basename lookup) in ROMPACK_ONLY builds.
    if (gw_pack::is_loaded()) {
        std::string base = path;
        size_t slash = base.find_last_of('/');
        if (slash != std::string::npos) {
            base = base.substr(slash + 1);
        }
        const uint8_t* bytes = nullptr;
        size_t size = 0;
        if (!base.empty() && gw_pack::get_file_bytes(base, bytes, size) && bytes && size > 0) {
            const bool ok = loadTextureFromMem(tex, NULL, bytes, size);
            if (!ok) {
                YOKOI_LOG("tex: import failed from pack '%s' (%u bytes)", base.c_str(), (unsigned)size);
            }
            return ok;
        }

        // We expected it in the pack but didn't find it.
        if (!base.empty()) {
            YOKOI_LOG("tex: missing in pack '%s' (from '%s')", base.c_str(), path.c_str());
        }
    }

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        YOKOI_LOG("tex: fopen failed '%s'", path.c_str());
        return false;
    } // not sucess

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* data = (uint8_t*)malloc(size);
    if (!data) { fclose(file); return false; } // not sucess

    fread(data, 1, size, file);
    fclose(file);

    const bool ok = loadTextureFromMem(tex, NULL, data, size);
    if (!ok) {
        YOKOI_LOG("tex: import failed from file '%s' (%u bytes)", path.c_str(), (unsigned)size);
    }
    free(data); // free ram
    return ok;
}



void Virtual_Screen::set_base_environnement()
{
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
}


void Virtual_Screen::set_alpha_environnement(uint8_t alpha_multiply){
    // Change color of texture
    // texture polygone use only alpha from texture and apply a constant color
    // Usefull for segment of game watch (texture contain only alpha, color choose by code)
    // Can multiply by alpha value to modify the global alpha value (created a slight transparency on segment)

    // (tips : storage only alpha info for segment is very usefull for have 8bit texture.
    //      3ds has not enough vram for contain lot of RGBA8 texture -> storage only A8 texture)
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
    C3D_TexEnv* env1 = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env1);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
    curr_alpha_color = SEGMENT_COLOR[0];
	C3D_TexEnvColor(env, RGBtoBGR8(curr_alpha_color) | (alpha_multiply<<24)); 
}

void Virtual_Screen::change_alpha_color_environnement(uint32_t color_, uint8_t alpha_multiply){
    // Change only constant color of texture 
    // -> usefull for crab and spyball G&W who contain segment with different color between segment
    if(curr_alpha_color == color_) { return; } // no change
    curr_alpha_color = color_;
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvColor(env, RGBtoBGR8(curr_alpha_color) | (alpha_multiply<<24)); 
}

void Virtual_Screen::set_color_environnement(uint32_t color){
    // Set constant color of polygone -> used for create uniform color background
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
    C3D_TexEnv* env1 = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env1);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, GPU_CONSTANT, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_CONSTANT, GPU_CONSTANT, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
	C3D_TexEnvColor(env, RGBtoBGR8(color)|0xFF000000); 
}



void Virtual_Screen::set_screen_up(bool on_left_eye){
    // Set on screen up -> tell to gpu "I want to write graphic for up screen"
    if(on_left_eye) { C3D_FrameDrawOn(target_up); }
    else { C3D_FrameDrawOn(target_right); }
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection_up); 
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView_up);
}


void Virtual_Screen::set_screen_down(){
    // Set on screen up -> tell to gpu "I want to write graphic for down screen"
    C3D_FrameDrawOn(target_down);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection_down); 
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView_down);
}


std::vector<vertex> polygone_sprite(float pos_x, float pos_y, float pos_z
                        , float size_x, float size_y
						, float x_texture, float y_texture
                        , float x_size_texture, float y_size_texture
						, float x_total_size_texture, float y_total_size_texture
                        , bool flip_x = false, bool flip_y = false)
{
    // Create a polygone sprite -> polygone who can use for show sprite 
    //                      -> 2d img align to screen and show a part of a sprite sheet
    // Compose of two polygones : 2 triangles

	float x_uv_pos_0 = ((float)x_texture) / ((float)x_total_size_texture);
	float y_uv_pos_0 = ((float)y_texture) / ((float)y_total_size_texture);
	float x_uv_size = ((float)x_size_texture) / ((float)x_total_size_texture);
	float y_uv_size = (((float)y_size_texture) / ((float)y_total_size_texture));
	float x_uv_pos_1 = x_uv_pos_0+x_uv_size;
	float y_uv_pos_1 = y_uv_pos_0+y_uv_size;

    if (flip_x){
        float tmp = x_uv_pos_0;
        x_uv_pos_0 = x_uv_pos_1;
        x_uv_pos_1 = tmp;
    }
    if(flip_y){
        float tmp = y_uv_pos_0;
        y_uv_pos_0 = y_uv_pos_1;
        y_uv_pos_1 = tmp;
    }

	return {
		// first triangle
		{ { pos_x, 		  pos_y, 		 pos_z }, {x_uv_pos_1,   y_uv_pos_1 } }, // bas-gauche
		{ { pos_x+size_x, pos_y,  		 pos_z }, {x_uv_pos_0,   y_uv_pos_1 } }, // bas-droite
		{ { pos_x+size_x, pos_y+size_y,  pos_z }, {x_uv_pos_0,   y_uv_pos_0  } }, // haut-droite
		// second triangle
		{ { pos_x+size_x, pos_y+size_y,  pos_z }, { x_uv_pos_0,  y_uv_pos_0 } }, // bas-droite
		{ { pos_x, 		  pos_y+size_y,  pos_z }, { x_uv_pos_1,  y_uv_pos_0 } }, // bas-gauche
		{ { pos_x, 		  pos_y,  		 pos_z }, { x_uv_pos_1,  y_uv_pos_1 } }, // haut-gauche
	};
}



////// Init and set parameter ////////////////////////////////////////////////////////////

void Virtual_Screen::config_screen(void){
    acInit();    
    gfxInitDefault();
    romfsInit();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // 3D stereoscopic activation -> always activate
    // Apparently, the 3DS automatically disables 3D
    // if only the left screen buffer is filled during a frame
    // (between C3D_FrameBegin(C3D_FRAME_SYNCDRAW) and C3D_FrameEnd(0)).
    // As a result, the emulator menu will automatically be displayed in 2D
    // (because only one buffer is filled via the set_text and set_img functions),
    // while compatible games will automatically display in 3D (both buffers filled).
    // (PS: Thanks Nintendo for automating this!)    
    gfxSet3D(true);

    // Create render target
    target_up = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(target_up, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    
    target_right = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(target_right, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

	target_down = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(target_down, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	// Load the vertex shader, create a shader program and bind it
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	C3D_BindProgram(&program);

	// Get the location of the uniforms
	uLoc_projection   = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView    = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
	
	// Configure attributes for use with the vertex shader
	C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord

	// calcul for go to 3d into 2d
	Mtx_OrthoTilt(&projection_up, 0.0f, 400.0f, 240.0f, 0.0f, 1.0f, -5.0f, true);
	Mtx_OrthoTilt(&projection_down, 0.0f, 320.0f, 240.0f, 0.0f, 1.0f, -5.0f, true);
	C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);

	// default for model view
	Mtx_Identity(&modelView_up);
	Mtx_Identity(&modelView_down);

    C3D_FVUnifSet(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(program.vertexShader, "uniColor"), 1, 1, 1, 1);

    if (!loadTexture_file("romfs:/gfx/texte_3ds.t3x", &text_texture)) {
        printf("Failed to load UI font texture: romfs:/gfx/texte_3ds.t3x\n");
    }
    if (!loadTexture_file("romfs:/gfx/noise.t3x", &noise_texture)) {
        printf("Failed to load noise texture: romfs:/gfx/noise.t3x\n");
    } else {
        C3D_TexSetWrap(&noise_texture, GPU_REPEAT, GPU_REPEAT);
    }
    set_base_environnement();

    vertex_data = (vertex*)linearAlloc((nb_text_max+nb_img_interface_max+nb_segments_max)*6*sizeof(vertex)+1000);
    index_start_texte = nb_segments_max*6;
    size_text_screen_0 = 0; size_text_screen_1 = 0;
    send_vbo();

    delete_all_text();
    set_screen_up();
}


bool Virtual_Screen::load_visual(std::string path_segment
                                , const Segment* segment_list, const size_t size_segment_list
                                , const uint16_t* v_segment_info
                                , std::string path_background 
                                , const uint16_t* v_background_info ){ 

    // load texture
    already_load_game = false;
    if (!loadTexture_file(path_segment, &texture_game)) {
        printf("Erreur chargement texture segment ! (%s)\n", path_segment.c_str());
        return false;
    }
    C3D_TexSetFilter(&texture_game, GPU_LINEAR, GPU_NEAREST);
    if(!path_background.empty()){
        if (!loadTexture_file(path_background, &background)) {
            printf("Erreur chargement texture background ! (%s)\n", path_background.c_str());
            img_background = false;
        } else {
            C3D_TexSetFilter(&background, GPU_LINEAR, GPU_NEAREST);
            img_background = true;
        }
    } else { img_background = false; }

    // Load segments (attributs) -> Sort for start with first screen and finish with second screen
    list_segment.clear();
    list_segment.resize(size_segment_list);
    background_ind_vertex.resize(0);
    nb_screen = 1;
    uint16_t curr_i_screen_0 = 0;
    uint16_t curr_i_screen_1 = size_segment_list-1; // size == size_segment_list so curr_i_secreen_1 == curr_i_screen_0 only at the end of loop
    for(size_t i = 0; i < size_segment_list; i++){
        if(segment_list[i].screen == 0){
            list_segment[curr_i_screen_0] = segment_list[i];
            curr_i_screen_0+=1;
        }
        else { // second screen = screen 1
            list_segment[curr_i_screen_1] = segment_list[i];
            curr_i_screen_1 -= 1;
            nb_screen = 2;
        }
    }
    index_segment_screen[0] = 0;
    index_segment_screen[1] = curr_i_screen_0;
    index_segment_screen[2] = curr_i_screen_1+1;
    index_segment_screen[3] = size_segment_list;

    // Load additional information
    segment_info = v_segment_info;
    is_mask = (segment_info[I_SEG_MASK]&0x01) == 0x01;
    double_in_one_screen = (segment_info[I_SEG_MASK]&0x02) == 0x02;
    background_info = v_background_info;

    // set color of fond
    if (is_mask){ 
        curr_fond_color = SEGMENT_COLOR[0]; 
        curr_fond_color_noise = lum_rgb8(curr_fond_color, _LUM_LIGHT_FOND_NOISE_) ;
    }
    else { 
        curr_fond_color = g_settings.background_color; 
        curr_fond_color_noise = lum_rgb8(curr_fond_color, _LUM_DARK_FOND_NOISE_);
    }
    
    already_load_game = true;
    return true;
}


void Virtual_Screen::refresh_settings(){
    if(background_info[i_camera(nb_screen)] == 1){
        cam.exit();
        cam.init();
    }

    // Update background color from settings (only if not using mask)
    if (!is_mask) {
        curr_fond_color = g_settings.background_color;
        curr_fond_color_noise = lum_rgb8(curr_fond_color, _LUM_DARK_FOND_NOISE_);
    }
    // Segment marking effect settings are already read from g_settings in update_screen()
}


bool Virtual_Screen::init_visual(){
    Segment seg_gw; std::vector<vertex> curr_vertex;
    uint32_t curr_index = 0;
    uint16_t decal_x = 0;
    
    // calculate value for screen are center
    int16_t align_x[nb_screen];
    int16_t align_y[nb_screen];

    pos_fond.clear(); 

    // X
    if(double_in_one_screen) { // set to edge of screen
        int8_t gap = 1;
        align_x[0] = min(0, (200-gap)-(int)segment_info[i_seg_scr_w(0)]); 
        align_x[1] = max(200-(int)segment_info[i_seg_scr_w(1)], gap); 
    } 
    else { for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){
            align_x[curr_screen] = int((_3DS_SCREEN_X[curr_screen] - segment_info[i_seg_scr_w(curr_screen)])/2);
    } }
    // Y
    if(nb_screen == 1 || double_in_one_screen){ 
        align_y[0] = int((_3DS_SCREEN_Y[0] - segment_info[i_seg_scr_h(0)])/2); 
        if(nb_screen != 1){ align_y[1] = align_y[0]; } // double in one screen
    } 
    else { align_y[0] = _3DS_SCREEN_Y[0] - segment_info[i_seg_scr_h(0)]; align_y[1] = 0; }

    // generate polygone fond
    for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){ // background

        if(curr_screen != 0 && double_in_one_screen){ decal_x = 200; }
        else { decal_x = 0; }

        pos_fond.push_back(align_x[curr_screen]+decal_x);
        pos_fond.push_back(align_y[curr_screen]);

        if(background_info[i_camera(nb_screen)] == 0){ // uniform fond with ditering
            curr_vertex = polygone_sprite(
                                align_x[curr_screen]+decal_x, align_y[curr_screen], -2
                                , background_info[i_bg_w(curr_screen)], background_info[i_bg_h(curr_screen)]
                                , 0, 0
                                , background_info[i_bg_w(curr_screen)], background_info[i_bg_h(curr_screen)]
                                , 64, 64);
        }
        else { // cam fond -> need to aligned texture
            curr_vertex = polygone_sprite(
                                align_x[curr_screen]+decal_x, align_y[curr_screen], -2
                                , background_info[i_bg_w(curr_screen)], background_info[i_bg_h(curr_screen)]//, 400, 240 //
                                , align_x[curr_screen]+decal_x, align_y[curr_screen]+(256-240)//, 0, 256-240
                                , background_info[i_bg_w(curr_screen)], background_info[i_bg_h(curr_screen)]//, 400, 240
                                , 512, 256, /*flip_x:*/ true);
        }
	    memcpy(&vertex_data[curr_index], curr_vertex.data(), 6*sizeof(vertex));
        background_ind_vertex.push_back(curr_index);
        curr_index += 6;
    }

    // generate polygone for image of background
    if(img_background){
        for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){ // background
            if(curr_screen != 0 && double_in_one_screen){ decal_x = 200; }
            else { decal_x = 0; }

            curr_vertex = polygone_sprite(
                                align_x[curr_screen]+decal_x, align_y[curr_screen], -1
                                , background_info[i_bg_w(curr_screen)], background_info[i_bg_h(curr_screen)]
                                , background_info[i_bg_x(curr_screen)], background_info[i_bg_y(curr_screen)]
                                , background_info[i_bg_w(curr_screen)], background_info[i_bg_h(curr_screen)]
                                , background_info[I_TEX_W], background_info[I_TEX_H]);
            memcpy(&vertex_data[curr_index], curr_vertex.data(), 6*sizeof(vertex));
            background_ind_vertex.push_back(curr_index);
            curr_index += 6;
        }  
    }

    // generate polygone for segments
    for(size_t i = 0; i < list_segment.size(); i++){ // segment
		seg_gw = list_segment[i];
        if(seg_gw.screen != 0 && double_in_one_screen){ decal_x = 200; }
        else { decal_x = 0; }

		curr_vertex = polygone_sprite(
								seg_gw.pos_scr[0]/segment_info[I_SEG_MULTI]+align_x[seg_gw.screen]+decal_x // pos x
                                , seg_gw.pos_scr[1]/segment_info[I_SEG_MULTI]+align_y[seg_gw.screen], 0 // pos y and z
								, seg_gw.size_tex[0]/segment_info[I_SEG_MULTI], seg_gw.size_tex[1]/segment_info[I_SEG_MULTI] // size x and y
								, seg_gw.pos_tex[0], seg_gw.pos_tex[1] // texture uv pos
								, seg_gw.size_tex[0], seg_gw.size_tex[1] // texture uv size
								, segment_info[I_TEX_W], segment_info[I_TEX_H]); // texture max size
	    memcpy(&vertex_data[curr_index], curr_vertex.data(), 6*sizeof(vertex));
        list_segment[i].index_vertex = curr_index;
        curr_index += 6;
        if(curr_index >= nb_segments_max*6){ break; }
    }


    if(background_info[i_camera(nb_screen)] == 1){
        cam.init();
    }

    send_vbo();
    return true;
}


////// Used for menu of emulateur ///////////////////////////////////////////////////////////////////////////

void Virtual_Screen::protect_blinking(Segment *seg, bool new_state){
    // Protect to "blinking segment"
    // On true G&W is not visible to segment blink (segment too slow to update)
    // but is visible on emulator without this protection
    
    // see before and new value of segment for patch if is only blink
    seg->state = seg->state && (new_state || seg->buffer_state);
    seg->state = seg->state || (new_state && seg->buffer_state);
}


bool Virtual_Screen::update_buffer_video(SM5XX* cpu){
    bool screen_are_update = false;
    if(!cpu->segments_state_are_update){ return screen_are_update; } // no need to update -> Stop
    
    for(size_t i_seg = 0; i_seg < list_segment.size(); i_seg++){
        Segment& curr_seg = list_segment[i_seg];
        bool new_state = cpu->get_segments_state(curr_seg.id[0], curr_seg.id[1], curr_seg.id[2]);
        protect_blinking(&curr_seg, new_state);
        if(curr_seg.buffer_state != curr_seg.state){ screen_are_update = true; }
        curr_seg.buffer_state = curr_seg.state;
        curr_seg.state = new_state;
    }
    cpu->segments_state_are_update = false;
    return screen_are_update;
}


void Virtual_Screen::set_text(const std::string& txt
                        , int16_t x_pos_init, int16_t y_pos
                        , uint8_t screen, uint8_t multiply_size){
    uint32_t pos_x = 0;
    for (char c : txt) {   
        if (size_text_screen_0+size_text_screen_1 >= nb_text_max) { break; } ; 
        if (letter_pos.find(c) != letter_pos.end()) {
            Coord letter_coor = letter_pos[c];
            std::vector<vertex> curr_vertex = polygone_sprite(
                x_pos_init+pos_x*8*multiply_size, y_pos, 0.2, 8*multiply_size, 8*multiply_size
                , 120-letter_coor.x*8, 120-letter_coor.y*8
                , 8, 8, 128, 128);
            if(screen == 0){ // beging of text space
                memcpy(&vertex_data[index_start_texte+size_text_screen_0*6], curr_vertex.data(), curr_vertex.size() * sizeof(vertex)); 
                size_text_screen_0+=1;
            }
            else { // end of text space
                memcpy(&vertex_data[index_start_texte +(nb_text_max-1-size_text_screen_1)*6], curr_vertex.data(), curr_vertex.size() * sizeof(vertex)); 
                size_text_screen_1+=1;
            }
        }
        pos_x += 1;
    }
    send_vbo();
}

void Virtual_Screen::delete_all_text(){
    size_text_screen_0 = 0; size_text_screen_1 = 0;
}

void Virtual_Screen::update_text(bool clean){
    set_alpha_environnement();
    C3D_TexBind(0, &text_texture);
    if(size_text_screen_0 != 0){
        set_screen_up(); 
        if(clean) {C3D_RenderTargetClear(target_up, C3D_CLEAR_ALL, (FOND_COLOR_MENU<<8)|0xFF, 0);}
        C3D_DrawArrays(GPU_TRIANGLES, index_start_texte, size_text_screen_0*6);
    }
    if(size_text_screen_1 != 0){
        set_screen_down(); 
        if(clean) {C3D_RenderTargetClear(target_down, C3D_CLEAR_ALL, (FOND_COLOR_MENU<<8)|0xFF, 0);}
        C3D_DrawArrays(GPU_TRIANGLES, index_start_texte +(nb_text_max-size_text_screen_1)*6, size_text_screen_1*6);
    }
}



void Virtual_Screen::set_img(const std::string& path, const uint16_t* info
                                , int16_t x_pos, int16_t y_pos, uint8_t img_i){
    // polygone
    std::vector<vertex> curr_vertex = polygone_sprite(
        x_pos, y_pos, -0.5, info[I_IMG_W], info[I_IMG_H]
        , info[I_IMG_X], info[I_IMG_Y], info[I_IMG_W], info[I_IMG_H]
        , info[I_TEX_W], info[I_TEX_H]);
    memcpy(&vertex_data[index_start_texte+(nb_text_max+img_i)*6], curr_vertex.data(), curr_vertex.size() * sizeof(vertex)); 
    send_vbo();
    // textyre
    const bool same_texture = (img_texture[img_i].data != nullptr && img_texture_path[img_i] == path);
    if (!same_texture) {
        if (img_texture[img_i].data) {
            C3D_TexDelete(&img_texture[img_i]);
            img_texture[img_i].data = nullptr;
        }
        if (!loadTexture_file(path, &img_texture[img_i])) {
            printf("Erreur chargement texture segment !\n");
            img_texture_path[img_i].clear();
        } else {
            img_texture_path[img_i] = path;
        }
    }
    // set index for update
    if (std::find(indice_img.begin(),indice_img.end(),img_i) == indice_img.end()) { indice_img.push_back(img_i); }
}

void Virtual_Screen::delete_all_img(){
    for (size_t img_i = 0; img_i < indice_img.size(); img_i++) { 
        if (img_texture[img_i].data) { C3D_TexDelete(&img_texture[img_i]); img_texture[img_i].data = nullptr; }
        img_texture_path[img_i].clear();
    }
    indice_img.resize(0);
}

void Virtual_Screen::update_img(bool clean){
    set_screen_down();
    set_base_environnement();
    if(clean){ C3D_RenderTargetClear(target_down, C3D_CLEAR_ALL, (FOND_COLOR_MENU<<8)|0xFF, 0); }
    for (size_t i = 0; i < indice_img.size(); i++) {  
        C3D_TexBind(0, &img_texture[indice_img[i]]);  
        C3D_DrawArrays(GPU_TRIANGLES, index_start_texte+(nb_text_max+indice_img[i])*6, 6);
    }
}




////// Generate G&W screen ///////////////////////////////////////////////////////////////////////////

int Virtual_Screen::set_good_screen(int curr_screen){
    // Set screen need at begining of generate game visual
    // In general, curr_screen = 0 -> screen top
    //              curr_screen = 1 -> screen bottown
    // Based to paramater of curr load game

    // Some Gamewatch double screen (Lifeboat, rainShower and mario bros) 
    // have all graphic on top screen (curr_screen = 1 -> screen top) 
    // parameter : double_in_one_screen  

    // Return nb render need for this screen (1 = 2d, 2 = 3d stereoscopic)
    if(curr_screen == 0) { // first screen -> up
        set_screen_up(); 
        C3D_RenderTargetClear(target_up, C3D_CLEAR_ALL, 0x000000, 0); 
        C3D_RenderTargetClear(target_right, C3D_CLEAR_ALL, 0x000000, 0); 
        modelView_curr = &modelView_up;
        return gfxIs3D() ? 2 : 1;
    }
    else if(!double_in_one_screen) { // second screen -> not move if double in one screen
        set_screen_down(); 
        C3D_RenderTargetClear(target_down, C3D_CLEAR_ALL, 0x000000, 0);
        modelView_curr = &modelView_down;
        return 1;
    } 
    else { // double_in_one_screen && curr_screen == 1
        // Not change used set screen => use previous config by change nothing
        // So not change use screen up and not clean this screen
    }
    return -1;
}



////// Update value used for 3d effect ///////////////////////////////////////////////////////////////////////////

void Virtual_Screen::update_slider_3d_value(int nb_render){
    if(nb_render == 1){ slider_3d = 0; return; }

    slider_3d = osGet3DSliderState();
    //slider_3d = (slider_3d-0.1f)*1.11f; // ( *1.11f ~= /0.9f )
    if(slider_3d <= 0){ slider_3d = 0; return; }
} 

void Virtual_Screen::update_eye_offset_value(int nb_render, int i_render){
    if(slider_3d == 0){ eye_offset_value = 0; return; }
    float orientation = (i_render == 0 ? 1.0f : -1.0f);
    eye_offset_value = slider_3d * orientation;
} 


void Virtual_Screen::update_offset_fond(){
    if(eye_offset_value != 0 && !is_mask){
        float coef_bg_in_front = -4.5f;
        offset_fond = eye_offset_value * coef_bg_in_front;
    }
    else { offset_fond = 0; }
}


void Virtual_Screen::update_offset_segment(){
    if(eye_offset_value == 0){ 
        offset_segment = 0;
        offset_mark_segment = 0;
        return;
    }

    float coef_bg_in_front ;
    if(is_mask){ 
        coef_bg_in_front = -3.0f; 
    }
    else {
        coef_bg_in_front = background_info[i_bg_in_front(nb_screen)] == 1 ? -3.0f: 1.0f;
    }
    offset_segment = eye_offset_value * coef_bg_in_front;
    offset_mark_segment = eye_offset_value * (coef_bg_in_front-1.0f);
}

void Virtual_Screen::update_offset_background(){
    if(eye_offset_value == 0){ 
        offset_background = 0; return; }

    float coef_bg_in_front = background_info[i_bg_in_front(nb_screen)] == 1 ? 0.0f: -3.0f; //1.65f: 0.8f;
    offset_background = eye_offset_value * coef_bg_in_front;
}


////// apply transformation for 3d effect ///////////////////////////////////////////////////////////////////////////

void Virtual_Screen::apply_3d_segment(C3D_Mtx* curr_modelView, int i_render, bool is_active){
    if(offset_segment == 0){ return; }

    float round_better = (i_render == 0 ? 0 : -0.5f);
    float result = round_better + (is_active) ? offset_segment: offset_mark_segment;

    Mtx_Translate(curr_modelView, (int)(result), 0, 0, true);
}


void Virtual_Screen::apply_3d_background(C3D_Mtx* curr_modelView, int i_render){
    if(offset_background == 0){ return; }
    float round_better = (i_render == 0 ? 0 : -0.5f);
    Mtx_Translate(curr_modelView, (int)(offset_background + round_better), 0, 0, true);
}


void Virtual_Screen::apply_3d_fond(C3D_Mtx* curr_modelView, int i_render, float zoom){
    if(offset_fond != 0 && !is_mask){
        float val_move = slider_3d*zoom;
        Mtx_Translate(curr_modelView, _3DS_SCREEN_X[0]*0.5f, _3DS_SCREEN_Y[0]*0.5f, 0, true);
        Mtx_Scale(curr_modelView, 1 - val_move, double_in_one_screen ? 1 : 1 - val_move, 1);
        Mtx_Translate(curr_modelView, -_3DS_SCREEN_X[0]*0.5f, -_3DS_SCREEN_Y[0]*0.5f, 0, true);
        
        float round_better = (i_render == 0 ? 0 : -0.5f);
        Mtx_Translate(curr_modelView, (int)(offset_fond + round_better), 0, 0, true);
    }
}


////// create graphics ///////////////////////////////////////////////////////////////////////////

void Virtual_Screen::create_fond(int nb_render, int i_render, int curr_screen){
    C3D_Mtx modelView_tmp;

    if(background_info[i_camera(nb_screen)] == 0){ 
        ////// Generate fond Color /////////////////////////////////////
        Mtx_Copy(&modelView_tmp, modelView_curr);
        apply_3d_fond(&modelView_tmp, i_render, -0.08f);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        set_color_environnement( (slider_3d == 0) ? curr_fond_color: curr_fond_color_noise);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);

        if(slider_3d != 0){ 
            // make two polygone -> fond with lower color and other with fond color.
            // Second polygone hav some "hole" for see lower color of other polygone
            // Usefull for 3d because uniform fond is complex to see in 3d
            set_alpha_environnement();
            change_alpha_color_environnement(curr_fond_color);
            C3D_TexBind(0, &noise_texture); // load texture
            C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);
        }
    }
    else {
        ////// Generate camera fond /////////////////////////////////////
        Mtx_Copy(&modelView_tmp, modelView_curr);
        apply_3d_fond(&modelView_tmp, i_render, -0.08f);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        set_base_environnement();
        if(i_render == 0){ C3D_TexBind(0, &(cam.tex_Left)); }
        else { C3D_TexBind(0, &(cam.tex_Left/*tex_Right*/)); }
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);

        set_alpha_environnement();
        change_alpha_color_environnement(curr_fond_color, 0x38);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);
    }

}


void Virtual_Screen::create_shadow(int nb_render, int i_render, int curr_screen){
    if(background_info[i_camera(nb_screen)] == 1){ return; } // no shadow if camera

    C3D_Mtx modelView_tmp;
    int decal = 0;
    int default_decal = 2;

    set_alpha_environnement();

    // Shadow Segments
    C3D_TexBind(0, &texture_game); // load texture
    change_alpha_color_environnement(0x111111, offset_segment == 0 ? 0x18 : 0x24);

    default_decal = (offset_fond != 0) ? 3 : ((background_info[i_bg_in_front(nb_screen)] == 1) ? 2: 3) ;

    decal = offset_segment - offset_fond;
    decal = (decal < 0) ? -decal : decal;
    decal = (int)(decal * _COEF_MOVE_SHADOW_);

    Mtx_Copy(&modelView_tmp, modelView_curr); // get current space of 3d model (Transformation Matrix)
    apply_3d_fond(&modelView_tmp, i_render, _ZOOM_SHADOW_);
    Mtx_Translate(&modelView_tmp, default_decal+decal, default_decal+decal, -0.20f, true); // Translate all model along x, y axis (and a little z)
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp); // transfert Transformation Matrix to gpu (to vertex shader)
    
    for(size_t i = index_segment_screen[curr_screen*2]; i < index_segment_screen[curr_screen*2+1]; i++){
        Segment seg_gw = list_segment[i];
        if(seg_gw.buffer_state){ C3D_DrawArrays(GPU_TRIANGLES, seg_gw.index_vertex, 6); }
    }


    // Shadow Background
    if(img_background && background_info[i_bg_shadow(nb_screen)] == 1){ 
        C3D_TexBind(0, &background);
        change_alpha_color_environnement(0x000000, 0x24);

        default_decal = (eye_offset_value != 0) ? 3 : ((background_info[i_bg_in_front(nb_screen)] == 1) ? 3: 2);

        decal = offset_background - offset_fond;
        decal = (decal < 0) ? -decal : decal;
        decal = (int)(decal * _COEF_MOVE_SHADOW_);

        Mtx_Copy(&modelView_tmp, modelView_curr);
        apply_3d_fond(&modelView_tmp, i_render, _ZOOM_SHADOW_);
        Mtx_Translate(&modelView_tmp, default_decal+decal, default_decal+decal, -0.10f, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);

        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[nb_screen+curr_screen], 6);
    }
}



void Virtual_Screen::create_segment(int nb_render, int i_render, int curr_screen){
    C3D_Mtx modelView_tmp;

    // Color Segment
    if(!is_mask){ set_alpha_environnement(); } // no mask segment = classic game watch -> set a color
    else { set_base_environnement(); } // mask segment = panorama screen and table top -> sprite has aready color

    C3D_TexBind(0, &texture_game); // load texture

    // only for Classic segment of Game & Watch (standars Game watch, not panorama screen or table top)
    if(!is_mask){ 
        // slight segment marking effect 
        Mtx_Copy(&modelView_tmp, modelView_curr); // get current space of 3d model (Transformation Matrix)
        apply_3d_segment(&modelView_tmp, i_render, false);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        change_alpha_color_environnement(0x101010, g_settings.segment_marking_alpha);
        C3D_DrawArrays(GPU_TRIANGLES
                        , list_segment[index_segment_screen[curr_screen*2]].index_vertex
                        , 6*(index_segment_screen[curr_screen*2+1]-index_segment_screen[curr_screen*2])); 
    }

    //   -> modify matrix for 3d effect
    Mtx_Copy(&modelView_tmp, modelView_curr); 
    apply_3d_segment(&modelView_tmp, i_render, true);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
    uint8_t alpha_segment = (background_info[i_camera(nb_screen)] == 1) ? 0xA0: 0xFF;
    // create true segment
    for(size_t i = index_segment_screen[curr_screen*2]; i < index_segment_screen[curr_screen*2+1]; i++){
        Segment seg_gw = list_segment[i];
        if(seg_gw.buffer_state){ // segment is activ / visible
            change_alpha_color_environnement(SEGMENT_COLOR[seg_gw.color_index], alpha_segment); // function already check if necessary, useful for Crab and spitball
            C3D_DrawArrays(GPU_TRIANGLES, seg_gw.index_vertex, 6); 
        }
    }
}


void Virtual_Screen::create_background(int nb_render, int i_render, int curr_screen){
    C3D_Mtx modelView_tmp;
    if(img_background){ 
        C3D_TexBind(0, &background);
        set_base_environnement();

        Mtx_Copy(&modelView_tmp, modelView_curr); 
        apply_3d_background(&modelView_tmp, i_render);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);

        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[nb_screen+curr_screen], 6);
    }
}

void Virtual_Screen::create_border(int nb_render, int i_render, int curr_screen){
    C3D_Mtx modelView_tmp;
    // Create black mask around color fond
    // -> very usefull with 3d stereoscopic effect
    // -> gives the illusion of seeing the game through a window

    if(slider_3d != 0){
        set_color_environnement(0x000000);

        // Up - > in two part
        Mtx_Copy(&modelView_tmp, modelView_curr);
        uint16_t w = background_info[i_bg_w(curr_screen)];
        uint16_t h = background_info[i_bg_h(curr_screen)];

        Mtx_Translate(&modelView_tmp, (w*0.5f)-10, -h, 0, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);

        Mtx_Translate(&modelView_tmp, -(w-20), 0, 0, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);

        // down - > in two part
        set_color_environnement(0x000000);                
        Mtx_Translate(&modelView_tmp, 0, 2*h, 0, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);
        
        Mtx_Translate(&modelView_tmp, w-20, 0, 0, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);
        
        // Left and Right
        if(!double_in_one_screen){
            int compense_background = 0;
            if(offset_background != 0){
                compense_background = (int)(offset_background);
                if(compense_background < 0){ compense_background = -compense_background; }
                compense_background = compense_background;
            }
            set_color_environnement(0x000000);                
            Mtx_Copy(&modelView_tmp, modelView_curr);
            Mtx_Translate(&modelView_tmp, -w+compense_background, 0, 0, true);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
            C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);

            Mtx_Copy(&modelView_tmp, modelView_curr);
            Mtx_Translate(&modelView_tmp, w-compense_background, 0, 0, true);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView_tmp);
            C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);                
        }
    }
}



////// loop of screen ///////////////////////////////////////////////////////////////////////////


void Virtual_Screen::update_screen(){
    int nb_render_to_make = 1; 
    
    for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){

        ////// Set 3ds screen /////////////////////////////////////
        int res = set_good_screen(curr_screen);
        if (res != -1) { nb_render_to_make = res; }

        update_slider_3d_value(nb_render_to_make);
        if(background_info[i_camera(nb_screen)] == 1){ cam.update(); }

        for(int i_render = 0; i_render < nb_render_to_make; i_render++){

            update_eye_offset_value(nb_render_to_make, i_render);
            update_offset_fond();
            update_offset_segment();
            update_offset_background();

            // Paralax effect -> choose what screen up need
            if(curr_screen == 0 || double_in_one_screen){
                if(i_render==0){ set_screen_up(true); } // left eye -> default buffer if screen in 2D
                else { set_screen_up(false);  } // right eye
            }

            create_fond(nb_render_to_make, i_render, curr_screen);
            create_shadow(nb_render_to_make, i_render, curr_screen);
            create_background(nb_render_to_make, i_render, curr_screen);
            create_segment(nb_render_to_make, i_render, curr_screen);
            create_border(nb_render_to_make, i_render, curr_screen);
        }
    }
}





////// Close screen ///////////////////////////////////////////////////////////////////////////

void Virtual_Screen::Quit_Game(){
    if(!already_load_game){ return; }
	C3D_TexDelete(&texture_game);
    if(img_background){ C3D_TexDelete(&background); }
    if(background_info[i_camera(nb_screen)] == 1){ cam.exit(); }
}

void Virtual_Screen::Exit(void)
{
    Quit_Game();
    C3D_TexDelete(&text_texture);
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);

    C3D_Fini();
    romfsExit();
	gfxExit();
}
