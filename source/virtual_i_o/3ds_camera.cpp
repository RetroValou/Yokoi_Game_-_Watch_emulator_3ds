
#include "virtual_i_o/3ds_camera.h"
#include "virtual_i_o/3ds_camera_lookup_table.h"



void Camera_3ds::init(){

    loop_x_tile = _3DS_SCREEN_X_TOP/8;
    if(loop_x_tile*8 != _3DS_SCREEN_X_TOP){ loop_x_tile +=1; }
    
    loop_y_tile = _3DS_SCREEN_Y_TOP/8;
    if(loop_y_tile*8 != _3DS_SCREEN_Y_TOP){ loop_y_tile +=1; }


    buf = (u8*)linearAlloc(CAM_SIZE * 2);
    buf_Left  = buf;
    //buf_Right = buf + CAM_SIZE;

    texBuf = (u16*)linearAlloc(512 * 256 * 2 * 2);
    texBufLeft = texBuf;
    //texBufRight = texBuf + 512 * 256 * 2;
    for (int i = 0; i < 512 * 256 * 2 * 2; i++) {
        texBuf[i] = 0xFFFF;
    }

    C3D_TexInit(&tex_Left, 512, 256, GPU_RGB565);
    C3D_TexSetFilter(&tex_Left, GPU_LINEAR, GPU_LINEAR);
    C3D_TexUpload(&tex_Left, texBufLeft);

    /*
    C3D_TexInit(&tex_Right, 512, 256, GPU_RGB565);
    C3D_TexSetFilter(&tex_Right, GPU_LINEAR, GPU_LINEAR);
    C3D_TexUpload(&tex_Right, texBufRight);
    */
    camInit();

    CAMU_SetSize(SELECT_OUT1/*_OUT2*/, SIZE_CTR_TOP_LCD, CONTEXT_A);
    CAMU_SetOutputFormat(SELECT_OUT1/*_OUT2*/, OUTPUT_RGB_565, CONTEXT_A);
    
    CAMU_GetMaxBytes(&chunk_size_transfert, CAM_W, CAM_H);
    CAMU_SetTransferBytes(PORT_BOTH, chunk_size_transfert, CAM_W, CAM_H);

    CAMU_Activate(SELECT_OUT1/*_OUT2*/);

    bool new3DS = false;
    CAMU_FrameRate fps = FRAME_RATE_10;
    Result res = APT_CheckNew3DS(&new3DS);
    if(R_SUCCEEDED(res)) {
        if(new3DS) { fps = FRAME_RATE_10/*15*/; } // New 3ds -> stable in 15fps
        else { fps = FRAME_RATE_10; } // Old 3ds -> lower speed 10fps
    }
    CAMU_SetFrameRate(SELECT_OUT1/*_OUT2*/, fps);
    
    CAMU_SetNoiseFilter(SELECT_OUT1/*_OUT2*/, false);
    CAMU_SetAutoExposure(SELECT_OUT1/*_OUT2*/, true);
    CAMU_SetAutoWhiteBalance(SELECT_OUT1/*_OUT2*/, true);

    CAMU_SetTrimming(PORT_CAM1, false);
    //CAMU_SetTrimming(PORT_CAM2, false);

    CAMU_ClearBuffer(PORT_CAM1/*PORT_BOTH*/);
	//CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2);
    CAMU_StartCapture(PORT_CAM1/*PORT_BOTH*/);

    CAMU_GetBufferErrorInterruptEvent(&event_LeftError, PORT_CAM1);
    //CAMU_GetBufferErrorInterruptEvent(&event_RightError, PORT_CAM2);
    
    capturePending = false;
    start_capture();
}


void Camera_3ds::start_capture(){
    if(capturePending) return;

    Result resLeft  = CAMU_SetReceiving(&event_finish[0],  buf_Left,  PORT_CAM1, CAM_SIZE, (s16)chunk_size_transfert);
    //Result resRight = CAMU_SetReceiving(&event_finish[1], buf_Right, PORT_CAM2, CAM_SIZE, (s16)chunk_size_transfert);
    if(R_SUCCEEDED(resLeft) /*&& R_SUCCEEDED(resRight)*/) {
        capturePending = true;
    } else {
        event_finish[0] = 0;
        //event_finish[1] = 0;
    }
}


void Camera_3ds::update(){

    if(svcWaitSynchronization(event_LeftError, 0) == 0
        /*|| svcWaitSynchronization(event_RightError, 0) == 0*/){
            // Camera fail -> Restart
        if(event_finish[0] != 0) { svcCloseHandle(event_finish[0]);  event_finish[0]  = 0; }
        //if(event_finish[1] != 0) { svcCloseHandle(event_finish[1]); event_finish[1] = 0; }
        capturePending = false;
    }
 
    if (!capturePending) { start_capture(); return; } // new capture

    int32_t sync_out = 0;
    if (capturePending) { // check current capture is finish
        if (svcWaitSynchronizationN(&sync_out, event_finish, 1/*2*/, true, /*10000000*/0) == 0) {

            svcCloseHandle(event_finish[0]);
            //svcCloseHandle(event_finish[1]);
            event_finish[0] = 0;
            //event_finish[1] = 0;

            capturePending = false;

            update_texture();
        }
    }
}


void Camera_3ds::update_texture() {
    int x_buf, y_buf, index_tex, index_buffer;
    int y_b3; int x_b3; int y_b6;
    int xb_yb6_6;

    for(int y_b = 0; y_b < loop_y_tile; y_b++){
        y_b3 = (y_b<<3); y_b6 = (y_b<<6);

        for(int x_b = 0; x_b < loop_x_tile; x_b++){
            x_b3 = (x_b<<3);
            xb_yb6_6 = ((x_b+y_b6)<<6);

            for(int p = 0; p < 32; p++) {
                x_buf = x_b3 + TILE_ORDER_X[p];
                y_buf = y_b3 + TILE_ORDER_Y[p]; // <<3 = *8
                index_tex = (p<<1) + xb_yb6_6; // <<6 = *64
                index_buffer = (x_buf + y_buf*_3DS_SCREEN_X_TOP)<<1; // <<1 = *2
                memcpy(&texBufLeft[index_tex], &buf_Left[index_buffer], 4);
                //memcpy(&texBufRight[index_tex], &buf_Right[index_buffer], 4);
            }
        }
    }
    C3D_TexUpload(&tex_Left, texBufLeft);
    //C3D_TexUpload(&tex_Right, texBufRight);
}


void Camera_3ds::exit()
{
    // stop recept
    if (capturePending) {
        int32_t sync_out = 0;
        svcWaitSynchronizationN(&sync_out, event_finish, 1/*2*/, true, 10000000);
        svcCloseHandle(event_finish[0]);
        //svcCloseHandle(event_finish[1]);
        event_finish[0] = 0;
        //event_finish[1] = 0;
        capturePending = false;
    }

    // stop capture
    CAMU_StopCapture(PORT_BOTH);

    // desactive cam
    CAMU_Activate(SELECT_NONE);
    camExit();

    // free buffer img
    if (buf) {
        linearFree(buf);
        buf = NULL;
    }

    C3D_TexDelete(&tex_Left);
    C3D_TexDelete(&tex_Right);
}