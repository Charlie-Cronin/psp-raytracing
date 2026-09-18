#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>

using std::make_shared;
using std::shared_ptr;

// constants
const float infinity_f = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

// util functions 

inline float degrees_to_radians(float degrees){
    return degrees * pi / 180.0f;
}

inline float random_float() {
    return std::rand()/(RAND_MAX + 1.0f);
}

inline float random_float(float min, float max){
    return min + (max-min)*random_float();
}

inline int random_int(int min, int max){
    return int(random_float(min,max+1));
}


// common headers 

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "interval.h"

#endif