#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <psppower.h>

#include "rtweekend.h"

#include "material.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"

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

#define SAMPLES 10
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
    //material definitions
    //auto material_ground = make_shared<lambertian>(color(0.8f, 0.8f, 0.0f));
    //auto material_center = make_shared<lambertian>(color(0.1f, 0.2f, 0.5f));
    //auto material_left = make_shared<metal>(color(0.8f, 0.8f, 0.8f), 0.0f);
    //auto material_right = make_shared<dielectric>(1.5f);
    //auto material_bubble = make_shared<dielectric>(1.0f / 1.5f);

    //world definitions
    //world.add(make_shared<sphere>(point3( 0, -100.5f,   -1.0f),   100,     material_ground));
    //world.add(make_shared<sphere>(point3( 0,       0,   -1.2f),   0.5f,     material_center));
    //world.add(make_shared<sphere>(point3(-1,       0,   -1.0f),   0.5f,     material_right));
    //world.add(make_shared<sphere>(point3(-1,       0,   -1.0f),   0.4f,     material_bubble));
    //world.add(make_shared<sphere>(point3( 1,       0,   -1.0f),   0.5f,     material_left));
    
    auto ground_material = make_shared<lambertian>(color(0.5f, 0.5f, 0.5f));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -6; a < 6; a++){
        for (int b = -6; b < 6; b++){
            auto choose_mat = random_float();
            point3 centre(a+0.9f*random_float(), 0.2,b + 0.9*random_float());

            if ((centre - point3(4,0.2f,0)).length() > 0.9f){
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.0f){
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(centre,0.2f,sphere_material));
                }
                if (choose_mat < 0.95f){
                    auto albedo = color::random(0.5,1);
                    auto fuzz = random_float(0,0.5f);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(centre,0.2f,sphere_material));
                }
                else{
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(centre,0.2f,sphere_material));
                }

            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0,1,0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4f,0.2f,0.1f));
    world.add(make_shared<sphere>(point3(-4,1,0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7f,0.6f,0.5f), 0.0);
    world.add(make_shared<sphere>(point3(4,1,0), 1.0, material3));



    camera cam;
    
    cam.image_width = IMAGE_WIDTH;
    cam.image_height = IMAGE_HEIGHT;
    cam.buffer_width = BUFFER_WIDTH;
    cam.samples_per_pixel = SAMPLES;
    cam.max_depth = MAX_DEPTH;

    cam.vfov = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat = point3(0,0,0);

    cam.defocus_angle = 0.6f;
    cam.focus_dist = 10.0f;

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