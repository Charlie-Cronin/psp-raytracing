#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <psppower.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
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

#define SCENE_COUNT 6

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


// this struct contains all of the options that can be changed in the menu
struct MenuOptions{
    int scene_index = 0;
    int res_scale = 2; // either 1 or 2, full or half
    int samples = 20;
    int max_depth = 25;
    bool prevent_sleep = true;
};

// create new instance of struct
MenuOptions options;

//render the menu using psp debug
void render_menu(int current_option){
    pspDebugScreenSetXY(0,0);
    pspDebugScreenSetBackColor(0xFF945757);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenClear();

    pspDebugScreenPrintf("-----------------------------------\n");
    pspDebugScreenPrintf("     PSP RAYTRACER - SETTINGS     \n");
    pspDebugScreenPrintf("-----------------------------------\n\n");

    const char* scene_names[] = {"Cornell Box", "Cornell Box (empty)", "Simple Light", "Marble", "RT Weekend", "3_Spheres"};
    const char* res_names[] = {"Full Res (480x272)", "Half Res (240x136)"};

    pspDebugScreenPrintf(" %s Scene: %s",     (current_option == 0) ? "->" : "  ", scene_names[options.scene_index]);
    pspDebugScreenPrintf("\n %s Resolution: %s",     (current_option == 1) ? "->" : "  ", res_names[options.res_scale-1]);
    pspDebugScreenPrintf("\n %s Samples: %d",     (current_option == 2) ? "->" : "  ", options.samples);
    pspDebugScreenPrintf("\n %s Depth: %d",     (current_option == 3) ? "->" : "  ", options.max_depth);
    pspDebugScreenPrintf("\n %s Prevent Sleep: %s",     (current_option == 4) ? "->" : "  ", options.prevent_sleep ? "ON" : "OFF");

    pspDebugScreenPrintf("\n\n\n\n\n\n\n\n\n\n");
    pspDebugScreenPrintf("\n-----------------------------------\n");
    pspDebugScreenPrintf(" Use D-PAD UP/DOWN to navigate.\n");
    pspDebugScreenPrintf(" Use D-PAD LEFT/RIGHT to change values.\n");
    pspDebugScreenPrintf(" Press (X) to Start Render.\n");

}

// the interactive layer ontop of the menu (redraws the print every time there is an update)
void interactive_menu(){
    SceCtrlData pad;
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    int selected_option = 0;
    bool menu_active = true;
    bool needs_redraw = true;

    while (menu_active) {
        if(needs_redraw){
            render_menu(selected_option);
            needs_redraw = false;
        }
        
        // read pad input 
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_UP){
            if(selected_option == 0){
                selected_option = 4;
            }
            else{
                selected_option--;
            }
            needs_redraw = true;
            //debounce
            sceKernelDelayThread(150000);
        }
        else if(pad.Buttons & PSP_CTRL_DOWN){
            if(selected_option == 4){
                selected_option = 0;
            }
            else{
                selected_option++;
            }
            needs_redraw = true;
            //debounce
            sceKernelDelayThread(150000);
        }
        else if(pad.Buttons & PSP_CTRL_LEFT){
            // scene
            if (selected_option == 0){
                if(options.scene_index == 0){
                    options.scene_index = SCENE_COUNT-1;
                }
                else{
                    options.scene_index--;
                }
            }
            // res
            else if (selected_option == 1){
                if (options.res_scale == 2 ) options.res_scale = 1;
                else if (options.res_scale == 1 ) options.res_scale = 2;
            }
            // samples
            else if (selected_option == 2){
                if (options.samples == 5){
                    options.samples = 100;
                }
                else{
                    options.samples -= 5;
                }
            }
            
            // depth
            else if (selected_option == 3){
                if (options.max_depth == 5){
                    options.max_depth = 100;
                }
                else{
                    options.max_depth -= 5;
                }
            }

            // prevent sleep
            else if (selected_option == 4){
                options.prevent_sleep = !options.prevent_sleep;
            }
            needs_redraw = true;
            //debounce
            sceKernelDelayThread(150000);
        }
        else if(pad.Buttons & PSP_CTRL_RIGHT){
            // scene
            if (selected_option == 0){
                if(options.scene_index == SCENE_COUNT-1){
                    options.scene_index = 0;
                }
                else{
                    options.scene_index++;
                }
            }
            // res
            else if (selected_option == 1){
                if (options.res_scale == 2 ) options.res_scale = 1;
                else if (options.res_scale == 1 ) options.res_scale = 2;
            }
            // samples
            else if (selected_option == 2){
                if (options.samples == 100){
                    options.samples = 5;
                }
                else{
                    options.samples += 5;
                }
            }
            
            // depth
            else if (selected_option == 3){
                if (options.max_depth == 100){
                    options.max_depth = 5;
                }
                else{
                    options.max_depth += 5;
                }
            }

            // prevent sleep
            else if (selected_option == 4){
                options.prevent_sleep = !options.prevent_sleep;
            }
            needs_redraw = true;
            //debounce
            sceKernelDelayThread(150000);
        }
        else if (pad.Buttons & PSP_CTRL_CROSS){
            
            menu_active = false;
            sceKernelDelayThread(200000);
        }

        sceDisplayWaitVblankStart();
    }

    pspDebugScreenClear();
}


void create(){
    hittable_list world;

    int img_w = (options.res_scale == 1) ? 480 : 240;
    int img_h = (options.res_scale == 1) ? 272 : 136;

    scenes scene = scenes(img_w, img_h, BUFFER_WIDTH, options.samples, options.max_depth);
    camera cam;

    switch (options.scene_index){
        case 0: world = scene.cornell_box_cubes2(cam); break;
        case 1: world = scene.cornell_box_empty(cam); break;
        case 2: world = scene.simple_light(cam); break;
        case 3: world = scene.perlin_scene(cam); break;
        case 4: world = scene.RT_Weekend(cam); break;
        case 5: world = scene.custom_3_balls(cam); break;
    }
    
    //world = scene.simple_light(cam);
    //world = scene.quads(cam);
    //world = scene.perlin_scene();
    //world = scene.RT_Weekend();


    cam.render(world,image,options.prevent_sleep);

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
    vertices[1].u = (float)(options.res_scale == 1) ? 480 : 240;
    vertices[1].v = (float)(options.res_scale == 1) ? 272 : 136 - 0.1f;
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
    

    interactive_menu();

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