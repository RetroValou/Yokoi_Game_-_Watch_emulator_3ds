#include <algorithm>

#include "virtual_i_o/3ds_screen.h"
#include "std/timer.h"

#include "vshader_shbin.h" // shader used
#include "virtual_i_o/texte_decoup.h"

constexpr uint16_t _3DS_SCREEN_X[2] = { 400, 320 };
constexpr uint16_t _3DS_SCREEN_Y[2] = { 240, 240 };


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


void Virtual_Screen::send_vbo(){
    // send vbo to gpu
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, vertex_data, sizeof(vertex), 2, 0x210);
}


bool loadTexture_file(std::string path, C3D_Tex* tex) {

    FILE* file = fopen(path.c_str(), "rb");
    if (!file) { return false; } // not sucess

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* data = (uint8_t*)malloc(size);
    if (!data) { fclose(file); return false; } // not sucess

    fread(data, 1, size, file);
    fclose(file);

    loadTextureFromMem(tex, NULL, data, size);
    free(data); // free ram
    return true;
}


void Virtual_Screen::config_screen(void){    
    gfxInitDefault();
    romfsInit();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    target_up = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetSetOutput(target_up, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
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
	C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
	C3D_CullFace(GPU_CULL_NONE);

	// default for model view
	Mtx_Identity(&modelView_up);
	Mtx_Identity(&modelView_down);

    C3D_FVUnifSet(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(program.vertexShader, "uniColor"), 1, 1, 1, 1);

    loadTexture_file("romfs:/gfx/texte_3ds.t3x", &text_texture);
    set_base_environnement();

    vertex_data = (vertex*)linearAlloc((nb_text_max+nb_img_interface_max+nb_segments_max)*6*sizeof(vertex)+1000);
    index_start_texte = nb_segments_max*6;
    size_text_screen_0 = 0; size_text_screen_1 = 0;
    send_vbo();

    delete_all_text();
    set_screen_up();
}


void Virtual_Screen::load_visual(std::string path_segment
                                , const Segment* segment_list, const size_t size_segment_list
                                , const uint16_t* v_segment_info
                                , std::string path_background 
                                , const uint16_t* v_background_info ){ 

    // load texture
    if (!loadTexture_file(path_segment, &texture_game)) { printf("Erreur chargement texture segment !\n"); }
    C3D_TexSetFilter(&texture_game, GPU_LINEAR, GPU_NEAREST);
    if(!path_background.empty()){
        if (!loadTexture_file(path_background, &background)) { printf("Erreur chargement texture !\n"); }
        C3D_TexSetFilter(&background, GPU_LINEAR, GPU_NEAREST);
        img_background = true;
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
    is_mask = (segment_info[3]&0x01) == 0x01;
    double_in_one_screen = (segment_info[3]&0x02) == 0x02;
    background_info = v_background_info;

    // set color of fond
    if (is_mask){ curr_fond_color = SEGMENT_COLOR[0]; }
    else {curr_fond_color = CLEAR_COLOR; }
    already_load_game = true;
}


std::vector<vertex> polygone_sprite(float pos_x, float pos_y, float pos_z, float size_x, float size_y
						, float x_texture, float y_texture, float x_size_texture, float y_size_texture
						, float x_total_size_texture, float y_total_size_texture)
{
	float x_uv_pos = ((float)x_texture) / ((float)x_total_size_texture);
	float y_uv_pos = ((float)y_texture) / ((float)y_total_size_texture);
	float x_uv_size = ((float)x_size_texture) / ((float)x_total_size_texture);
	float y_uv_size = (((float)y_size_texture) / ((float)y_total_size_texture));
	return {
		// first triangle
		{ { pos_x, 		  pos_y, 		 pos_z }, {x_uv_pos+x_uv_size, y_uv_pos+y_uv_size	} }, // bas-gauche
		{ { pos_x+size_x, pos_y,  		 pos_z }, {x_uv_pos, 		   y_uv_pos+y_uv_size	} }, // bas-droite
		{ { pos_x+size_x, pos_y+size_y,  pos_z }, {x_uv_pos, 		   y_uv_pos 			} }, // haut-droite
		// second triangle
		{ { pos_x+size_x, pos_y+size_y,  pos_z }, { x_uv_pos, 			y_uv_pos 			} }, // bas-droite
		{ { pos_x, 		  pos_y+size_y,  pos_z }, { x_uv_pos+x_uv_size, y_uv_pos			} }, // bas-gauche
		{ { pos_x, 		  pos_y,  		 pos_z }, { x_uv_pos+x_uv_size, y_uv_pos+y_uv_size 	} }, // haut-gauche
	};
}


bool Virtual_Screen::init_visual(){
    Segment seg_gw; std::vector<vertex> curr_vertex;
    uint32_t curr_index = 0;
    uint16_t decal_x = 0;

    // calculate value for screen are center
    int16_t align_x[nb_screen];
    int16_t align_y[nb_screen];

    // X
    if(double_in_one_screen) { // set to edge of screen
        int8_t gap = 1;
        align_x[0] = min(0, (200-gap)-(int)segment_info[4]); 
        align_x[1] = max(200-(int)segment_info[6], gap); 
    } 
    else { for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){
            align_x[curr_screen] = int((_3DS_SCREEN_X[curr_screen] - segment_info[4+curr_screen*2])/2);
    } }
    // Y
    if(nb_screen == 1 || double_in_one_screen){ 
        align_y[0] = int((_3DS_SCREEN_Y[0] - segment_info[4+1])/2); 
        if(nb_screen != 1){ align_y[1] = align_y[0]; } // double in one screen
    } 
    else { align_y[0] = _3DS_SCREEN_Y[0] - segment_info[4+1]; align_y[1] = 0; }

    // generate polygone fond
    for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){ // background
        uint16_t id_src = 2 + curr_screen*4;
        if(curr_screen != 0 && double_in_one_screen){ decal_x = 200; }
        else { decal_x = 0; }

        curr_vertex = polygone_sprite(
                            align_x[curr_screen]+decal_x, align_y[curr_screen], -2
                            , background_info[2+id_src], background_info[3+id_src]
                            , 1, 1, 1, 1, 1, 1);
	    memcpy(&vertex_data[curr_index], curr_vertex.data(), 6*sizeof(vertex));
        background_ind_vertex.push_back(curr_index);
        curr_index += 6;
    }

    // generate polygone for image of background
    if(img_background){
        for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){ // background
            uint16_t id_src = 2 + curr_screen*4;
            if(curr_screen != 0 && double_in_one_screen){ decal_x = 200; }
            else { decal_x = 0; }

            curr_vertex = polygone_sprite(
                                align_x[curr_screen]+decal_x, align_y[curr_screen], -1
                                , background_info[2+id_src], background_info[3+id_src]
                                , background_info[0+id_src], background_info[1+id_src]
                                , background_info[2+id_src], background_info[3+id_src]
                                , background_info[0], background_info[1]);
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
								seg_gw.pos_scr[0]/segment_info[2]+align_x[seg_gw.screen]+decal_x // pos x
                                , seg_gw.pos_scr[1]/segment_info[2]+align_y[seg_gw.screen], 0 // pos y and z
								, seg_gw.size_tex[0]/segment_info[2], seg_gw.size_tex[1]/segment_info[2] // size x and y
								, seg_gw.pos_tex[0], seg_gw.pos_tex[1] // texture uv pos
								, seg_gw.size_tex[0], seg_gw.size_tex[1] // texture uv size
								, segment_info[0], segment_info[1]); // texture max size
	    memcpy(&vertex_data[curr_index], curr_vertex.data(), 6*sizeof(vertex));
        list_segment[i].index_vertex = curr_index;
        curr_index += 6;
        if(curr_index >= nb_segments_max*6){ break; }
    }

    send_vbo();
    return true;
}


void Virtual_Screen::protect_blinking(Segment *seg, bool new_state){
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

void Virtual_Screen::set_screen_up(){
    C3D_FrameDrawOn(target_up);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection_up); 
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView_up);
}

void Virtual_Screen::set_screen_down(){
    C3D_FrameDrawOn(target_down);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection_down); 
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView_down);
}


void Virtual_Screen::set_text(const std::string& txt, int16_t x_pos_init, int16_t y_pos, uint8_t screen, uint8_t multiply_size){
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
        x_pos, y_pos, -0.5, info[4], info[5]
        , info[2], info[3], info[4], info[5]
        , info[0], info[1]);
    memcpy(&vertex_data[index_start_texte+(nb_text_max+img_i)*6], curr_vertex.data(), curr_vertex.size() * sizeof(vertex)); 
    send_vbo();
    // textyre
    if (img_texture[img_i].data) { C3D_TexDelete(&img_texture[img_i]); img_texture[img_i].data = nullptr;  }    
    if (!loadTexture_file(path, &img_texture[img_i])) { printf("Erreur chargement texture segment !\n"); }
    // set index for update
    if (std::find(indice_img.begin(),indice_img.end(),img_i) == indice_img.end()) { indice_img.push_back(img_i); }
}

void Virtual_Screen::delete_all_img(){
    for (size_t img_i = 0; img_i < indice_img.size(); img_i++) { 
        if (img_texture[img_i].data) { C3D_TexDelete(&img_texture[img_i]); img_texture[img_i].data = nullptr;  }    
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

void Virtual_Screen::update_screen(){
    C3D_Mtx* model_curr;
    C3D_Mtx model_tmp;

    for(int curr_screen = 0; curr_screen < nb_screen; curr_screen++){
        if(curr_screen == 0) { 
            set_screen_up(); 
            C3D_RenderTargetClear(target_up, C3D_CLEAR_ALL, 0x000000, 0); 
            model_curr = &modelView_up; 
        }
        else if(!double_in_one_screen) { // second screen -> not move if double in one screen
            set_screen_down(); 
            C3D_RenderTargetClear(target_down, C3D_CLEAR_ALL, 0x000000, 0);
            model_curr = &modelView_down; 
        } 

        // Fond 
        set_color_environnement(curr_fond_color);
        C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[curr_screen], 6);

        // decors
        if(img_background){ 
            C3D_TexBind(0, &background);
            if(background_info[2+nb_screen*4] == 1){
                // shadow 
                Mtx_Copy(&model_tmp, model_curr);
                Mtx_Translate(&model_tmp, 3, 3, -0.10f, true);
                C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &model_tmp);
                set_alpha_environnement();
                change_alpha_color_environnement(0x000000, 0x24);
                C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[nb_screen+curr_screen], 6);
            }
            // true_decors 
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, model_curr);
            set_base_environnement();
            C3D_DrawArrays(GPU_TRIANGLES, background_ind_vertex[nb_screen+curr_screen], 6);
        }

        // segment
        if(!is_mask){ set_alpha_environnement(); } // no mask segment, if mask segment = segment are color -> keep base_environement
        else { set_base_environnement(); }
        C3D_TexBind(0, &texture_game);

        if(!is_mask){
            // slight segment marking effect 
            Mtx_Copy(&model_tmp, model_curr);
            Mtx_Translate(&model_tmp, 0, 0, -0.40f, true);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &model_tmp);
            change_alpha_color_environnement(0x101010, 0x04);
            C3D_DrawArrays(GPU_TRIANGLES
                            , list_segment[index_segment_screen[curr_screen*2]].index_vertex
                            , 6*index_segment_screen[curr_screen*2+1]); 

            // shadow 
            Mtx_Copy(&model_tmp, model_curr);
            Mtx_Translate(&model_tmp, 2, 2, -0.20f, true);
            C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &model_tmp);
            change_alpha_color_environnement(0x111111, 0x18);
            for(size_t i = index_segment_screen[curr_screen*2]; i < index_segment_screen[curr_screen*2+1]; i++){
                Segment seg_gw = list_segment[i];
                if(seg_gw.buffer_state){ C3D_DrawArrays(GPU_TRIANGLES, seg_gw.index_vertex, 6); }
            }
        }

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, model_curr);
        for(size_t i = index_segment_screen[curr_screen*2]; i < index_segment_screen[curr_screen*2+1]; i++){
            Segment seg_gw = list_segment[i];
            if(seg_gw.buffer_state){ 
                change_alpha_color_environnement(SEGMENT_COLOR[seg_gw.color_index]); // function already check if necessary
                C3D_DrawArrays(GPU_TRIANGLES, seg_gw.index_vertex, 6); 
            }
        }
    }
}


void Virtual_Screen::set_base_environnement(){
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
}
void Virtual_Screen::set_alpha_environnement(uint8_t alpha_multiply){
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
    curr_alpha_color = SEGMENT_COLOR[0];
	C3D_TexEnvColor(env, RGBtoBGR8(curr_alpha_color) | (alpha_multiply<<24)); 
}
void Virtual_Screen::change_alpha_color_environnement(uint32_t color_, uint8_t alpha_multiply){
    if(curr_alpha_color == color_) { return; } // no change
    curr_alpha_color = color_;
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvColor(env, RGBtoBGR8(curr_alpha_color) | (alpha_multiply<<24)); 
}

void Virtual_Screen::set_color_environnement(uint32_t color){
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, GPU_CONSTANT, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_CONSTANT, GPU_CONSTANT, GPU_CONSTANT);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
	C3D_TexEnvColor(env, RGBtoBGR8(color)|0xFF000000); 
}

void Virtual_Screen::Quit_Game(){
    if(!already_load_game){ return; }
	C3D_TexDelete(&texture_game);
    if(img_background){ C3D_TexDelete(&background); }
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
