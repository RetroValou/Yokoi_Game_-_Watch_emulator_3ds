
#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>



constexpr uint16_t CAM_W = 400;
constexpr uint16_t CAM_H = 240;
constexpr uint16_t CAM_BPP = 2;
constexpr uint32_t CAM_SIZE = (CAM_W * CAM_H * CAM_BPP);

constexpr uint16_t TEX_CAM_X = 512;
constexpr uint16_t TEX_CAM_Y = 256;

constexpr uint16_t _3DS_SCREEN_X_TOP = 400;
constexpr uint16_t _3DS_SCREEN_Y_TOP = 240;


class Camera_3ds {
    public: 
        C3D_Tex tex_Left, tex_Right;

    private : 
        Handle event_finish[1/*2*/];   
        Handle event_LeftError;
        //Handle event_RightError;
      
        bool capturePending = false;

        u8* buf;
        u8* buf_Left;
        //u8* buf_Right;
        u32 chunk_size_transfert;

        u16* texBuf; 
        u16* texBufLeft; 
        //u16* texBufRight;

        uint16_t loop_x_tile;
        uint16_t loop_y_tile;


    public : 
        void init();
        void exit();
        void update();
        void show_img(int i_render);

    private : 
        void start_capture();
        void update_texture();
};
