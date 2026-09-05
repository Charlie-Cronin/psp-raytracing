#include <pspuser.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspgu.h>

// PSP_MODULE_INFO IS REQUIRED 
// name attributes major version minor version
PSP_MODULE_INFO("Raytracing-In-One-Weekend", 0, 1, 0);
// starts the thread in user mode
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

#define BUFFER_WIDTH 512 //same width as vram
#define BUFFER_HEIGHT 272
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT BUFFER_HEIGHT

// render scale (later move to camera.h when done)
#define IMAGE_WIDTH 256
#define IMAGE_HEIGHT 128

// display list
char list[0x20000] __attribute__((aligned(64)));
// image buffer
// stored as uint32_t as that is what the texture buffer in the tutorial is stored as
// needs to be 16 bit alligned 
uint32_t __attribute__((aligned(16))) image[IMAGE_WIDTH * IMAGE_HEIGHT];
int running;

// from texture implimentations
typedef struct
{
    float u, v;
    uint32_t colour;
    float x, y, z;
} TextureVertex;


int exit_callback(int arg1, int arg2, void *common){
    //sceKernelExitGame();
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

void render(){
    for (int j = 0; j < IMAGE_HEIGHT; j++){
        for (int i = 0; i < IMAGE_WIDTH; i++){
            // main loop
            float r = (float)i / (IMAGE_WIDTH-1);
            float g = (float)j / (IMAGE_HEIGHT-1);
            float b = 0.0f;

            int ir = (int)(255.999f * r);
            int ig = (int)(255.999f * g);
            int ib = (int)(255.999f * b);

            // create an array IMAGE_WIDTH pixels long for each j value
            //ARGB
            image[j*IMAGE_WIDTH+i] = (0xFF << 24) | (ib << 16) | (ig << 8) | ir;
        }
    }

    // write image to Gu Memory
    sceKernelDcacheWritebackInvalidateAll();
}

void drawToScreen(){
    static TextureVertex vertices[2];

    // top left of texture
    vertices[0].u = 0.0f;
    vertices[0].v = 0.0f;
    vertices[0].colour = 0xFFFFFFFF;
    vertices[0].x = 0.0f; //takes up entire screen so range is top left -> bottom right of screen
    vertices[0].y = 0.0f;
    vertices[0].z = 0.0f;

    // bottom right of texture
    vertices[1].u = IMAGE_WIDTH;
    vertices[1].v = IMAGE_HEIGHT;
    vertices[1].colour = 0xFFFFFFFF;
    vertices[1].x = SCREEN_WIDTH;  //takes up entire screen so range is top left -> bottom right of screen
    vertices[1].y = SCREEN_HEIGHT;
    vertices[1].z = 0.0f;

    sceGuTexMode(GU_PSM_8888, 0, 0, GU_FALSE);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
    // use rendered array as texture source
    sceGuTexImage(0,IMAGE_WIDTH,IMAGE_HEIGHT,IMAGE_WIDTH,image);

    sceGuEnable(GU_TEXTURE_2D);
    sceGuDrawArray(GU_SPRITES, GU_COLOR_8888 | GU_TEXTURE_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 2, 0, vertices);
    sceGuDisable(GU_TEXTURE_2D);
}


int main(void){
    // use above functions to make exiting possible
    setup_callbacks();
    initGu();

    // render before main loop so the raytracing doesnt occur every frame
    render();

    running = 1;
    while(running) {
        // start 
        sceGuStart(GU_DIRECT, list);
        sceGuClearColor(0xFF26170D);
        sceGuClear(GU_COLOR_BUFFER_BIT);

        drawToScreen();

        sceGuFinish();
        sceGuSync(0,0);
        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();

    }

    endGu();

    return 0;
}