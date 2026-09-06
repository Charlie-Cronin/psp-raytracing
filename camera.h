#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"

class camera{
    public:
        //public params here
        int image_width;
        int image_height;

        void render(const hittable& world, uint32_t* out){
            uint32_t __attribute__((aligned(16))) image[image_width * image_height];
            initialize();

            for (int j = 0; j < image_height; j++){
                std::string msg = "Rendering Scene...\n" + std::to_string(j) + "/" + std::to_string(image_height) + " lines done";
                pspDebugScreenSetXY(0, 0);
                pspDebugScreenPrintf("%s\n", msg.c_str());
                for (int i = 0; i < image_width; i++){
                    auto pixel_centre = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                    auto ray_direction = pixel_centre - centre;

                    ray r(centre,ray_direction);

                    color pixel_color = ray_color(r, world);

                    out[j*image_width+i] = write_color(pixel_color);
                }
            }
        }

    private:
        //private params here
        point3 centre; 
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;

        //american spelling :(
        void initialize(){
            float focal_length = 1.0f;
            float viewport_height = 2.0;
            float viewport_width = viewport_height * ((float)(image_width)/image_height);

            centre = point3(0,0,0);

            auto viewport_u = vec3(viewport_width,0,0);
            auto viewport_v = vec3(0, -viewport_height, 0);

            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            auto viewport_upper_left = centre - vec3(0,0,focal_length) - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        }

        color ray_color(const ray& r, const hittable& world) const{
            hit_record rec;

            if (world.hit(r, interval(0, infinity_f), rec)) {
                return 0.5f * (rec.normal + color(1,1,1));
            }

            vec3 unit_direction = unit_vector(r.direction());
            auto a = 0.5*(unit_direction.y()+ 1.0);
            return (1.0-a)*color(1.0,1.0,1.0) + a*color(0.5,0.7,1.0);
        }


};

#endif