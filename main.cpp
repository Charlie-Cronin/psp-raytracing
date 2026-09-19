#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <psppower.h>
#include <pspiofilemgr.h>
#include <cstring> 

#include "rtweekend.h"

#include "material.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "bvh.h"
#include "texture.h"
#include "scenes.h"

// PSP_MODULE_INFO IS REQUIRED 
// name attributes major version minor version
PSP_MODULE_INFO("Raytracing", 0, 1, 0);
// starts the thread in user mode
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU | THREAD_ATTR_USER);

//#define BUFFER_WIDTH 512 //same width as vram
#define BUFFER_WIDTH 512 
#define BUFFER_HEIGHT 272
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT BUFFER_HEIGHT

// render scale (later move to camera.h when done)
//#define IMAGE_WIDTH 256
//#define IMAGE_HEIGHT 128

//#define IMAGE_WIDTH 480
//#define IMAGE_HEIGHT 272

#define IMAGE_WIDTH 240
#define IMAGE_HEIGHT 136

//#define SAMPLES 25
//#define MAX_DEPTH 40

#define SAMPLES 20
#define MAX_DEPTH 25

// display list
char list[0x20000] __attribute__((aligned(64)));
// image buffer
// stored as uint32_t as that is what the texture buffer in the tutorial is stored as
// needs to be 16 bit alligned 
uint32_t __attribute__((aligned(16))) image[BUFFER_WIDTH * 512];
int running;

// from texture implimentations
typedef struct
{
    float u, v;
    uint32_t colour;
    float x, y, z;
} TextureVertex;

void log_msg(const char* msg) {
    SceUID fd = sceIoOpen("ms0:/raytrace_log.txt", PSP_O_WRONLY | PSP_O_APPEND | PSP_O_CREAT, 0777);
    if (fd >= 0) {
        sceIoWrite(fd, msg, strlen(msg));
        sceIoClose(fd);
    }
}

int exit_callback(int arg1, int arg2, void *common){
    sceKernelExitGame();
    running = 0;
    return 0;
}

int callback_thread(SceSize args, void *common){
    // sceKernelCreateCallback(name, function, argument)
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    // registers the exit callback and makes the home button active
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

PSP_MAIN_THREAD_STACK_SIZE_KB(512);


int setup_callbacks(void){
    
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0,0,0);
    if (thid >= 0){
        // thread_id, length of data in bytes, pointer to arguments 
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}

void initGu(){
    sceGuInit();

    //set up buffers
    sceGuStart(GU_DIRECT, list);
    // pixel format, vram pointer, frame bugger
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUFFER_WIDTH);
    sceGuDispBuffer(SCREEN_WIDTH,SCREEN_HEIGHT,(void*)0x88000,BUFFER_WIDTH);
    sceGuDepthBuffer((void*)0x110000, BUFFER_WIDTH);

    //set up a viewport 
    sceGuOffset(2048 - (SCREEN_WIDTH/2), 2048 - (SCREEN_HEIGHT/2));
    sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuScissor(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);

    //set some stuff
    sceGuDisable(GU_DEPTH_TEST); // disable the depth test as its not needed for a 2d image

    sceGuFinish();
    sceGuDisplay(GU_TRUE);
}

void endGu(){
    sceGuDisplay(GU_FALSE);
    sceGuTerm();
}





void create(){
    hittable_list world;
    scenes scene = scenes(IMAGE_WIDTH, IMAGE_HEIGHT, BUFFER_WIDTH, SAMPLES, MAX_DEPTH);
    camera cam;

    world = scene.cornell_box_cubes2(cam);
    //world = scene.simple_light(cam);
    //world = scene.quads(cam);
    //world = scene.perlin_scene();
    //world = scene.RT_Weekend();


    cam.render(world,image);

    // write image to Gu Memory
    sceKernelDcacheWritebackRange(image, sizeof(image));
    sceKernelDcacheWritebackInvalidateAll();
}








void drawToScreen(){
    static TextureVertex vertices[2];

    float margin_y = 16.0f;

    // top left of texture
    vertices[0].u = 0.0f;
    vertices[0].v = 0.0f;
    vertices[0].colour = 0xFFFFFFFF;
    vertices[0].x = 0.0f; //takes up entire screen so range is top left -> bottom right of screen
    vertices[0].y = 0.0f;
    vertices[0].z = 0.0f;

    // bottom right of texture
    vertices[1].u = (float)IMAGE_WIDTH;
    vertices[1].v = (float)IMAGE_HEIGHT - 0.1f;
    vertices[1].colour = 0xFFFFFFFF;
    vertices[1].x = (float)SCREEN_WIDTH;  //takes up entire screen so range is top left -> bottom right of screen
    vertices[1].y = (float)SCREEN_HEIGHT;
    vertices[1].z = 0.0f;

    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);

    sceGuTexWrap(GU_CLAMP, GU_CLAMP);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);


    // use rendered array as texture source
    void* uncached_image = (void*)((uint32_t)image | 0x40000000);
    sceGuTexImage(0,BUFFER_WIDTH,512,BUFFER_WIDTH,uncached_image);

    sceGuEnable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, vertices);
    sceGuDisable(GU_TEXTURE_2D);
}


int main(void){
    // use above functions to make exiting possible
    setup_callbacks();
    pspDebugScreenInit();
    pspDebugScreenSetXY(0,0);
    pspDebugScreenPrintf("Rendering Scene...\n");


    // render before main loop so the raytracing doesnt occur every frame
    create();

    initGu();

    running = 1;
    while(running) {
        // start 
        //scePowerTick(PSP_POWER_TICK_ALL);

        sceGuStart(GU_DIRECT, list);
        sceGuClearColor(0xFF000000);
        sceGuClear(GU_COLOR_BUFFER_BIT);

        drawToScreen();

        sceGuFinish();
        sceGuSync(0,0);
        sceDisplayWaitVblankStartCB();
        sceKernelDelayThread(10000);
        sceGuSwapBuffers();

    }

    endGu();

    return 0;
}