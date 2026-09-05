#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"

using color = vec3;

inline uint32_t __attribute__((always_inline)) write_color(const color& pixel_color){
    // main loop
    float r = pixel_color.x();
    float g = pixel_color.y();
    float b = pixel_color.z();

    int rbyte = (int)(255.999f * r);
    int gbyte = (int)(255.999f * g);
    int bbyte = (int)(255.999f * b);

    // create an array IMAGE_WIDTH pixels long for each j value
    //ARGB
    uint32_t packed = (0xFFu << 24) | (bbyte << 16) | (gbyte << 8) | rbyte;

    pspDebugScreenSetXY(0, 0);
    
    // no clue why but the program does not run on the psp without this, the text doesnt even display over the texture
    //pspDebugScreenPrintf("packed=0x%08X\n", (unsigned int)packed);

    return packed;
    
}

#endif