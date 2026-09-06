#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera{
    public:
        //public params here
        int image_width;
        int image_height;
        int buffer_width = 512;
        int samples_per_pixel = 25;
        int max_depth = 10;

        void render(const hittable& world, uint32_t* out){
            initialize();

            for (int j = 0; j < image_height; j++){
                std::string msg = "Rendering Scene...\n" + std::to_string(j) + "/" + std::to_string(image_height) + " lines done";
                pspDebugScreenSetXY(0, 0);
                pspDebugScreenPrintf("%s\n", msg.c_str());
                sceKernelDelayThread(1000);
                for (int i = 0; i < image_width; i++){
                    color pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; sample++){
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r,max_depth, world);
                    }
                    //auto pixel_centre = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                    //auto ray_direction = pixel_centre - centre;

                    //ray r(centre,ray_direction);

                    out[j*buffer_width+i] = write_color(pixel_color * pixel_samples_scale);
                }
            }
        }

    private:
        //private params here
        float aspect_ratio;
        point3 centre; 
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        float pixel_samples_scale;

        //american spelling :(
        void initialize(){
            float focal_length = 1.0f;
            float viewport_height = 2.0;
            aspect_ratio = 480.0f / 272.0f;
            float viewport_width = viewport_height * aspect_ratio;

            pixel_samples_scale = 1.0 / samples_per_pixel;

            centre = point3(0,0,0);

            auto viewport_u = vec3(viewport_width,0,0);
            auto viewport_v = vec3(0, -viewport_height, 0);

            pixel_delta_u = viewport_u / image_width;
            pixel_delta_v = viewport_v / image_height;

            auto viewport_upper_left = centre - vec3(0,0,focal_length) - viewport_u/2 - viewport_v/2;
            pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
        }

        ray get_ray(int i, int j) const {
            // construct a camera ray originating from the origin and directed at randomly sampled points around the pixel location
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_u) +  ((j + offset.y()) * pixel_delta_v);

            auto ray_origin = centre;
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        vec3 sample_square() const{
            return vec3(random_float() - 0.5f, random_float() -0.5f, 0);
        }

        // use for loop instead of recurrsive function to save psp memory 
        color ray_color(const ray& r_in, int depth, const hittable& world) const {
            ray current_ray = r_in;
            color accumulated_attenuation(1.0f, 1.0f, 1.0f);

            for (int d = 0; d < depth; d++) {
                hit_record rec;

                if (world.hit(current_ray, interval(0.001f, infinity_f), rec)) {
                    ray scattered;
                    color attenuation;

                    if (rec.mat->scatter(current_ray, rec, attenuation, scattered)) {
                        accumulated_attenuation = accumulated_attenuation * attenuation;
                        current_ray = scattered;
                    } else {
                        return color(0, 0, 0);
                    }
                } else {
                    // Skybox hit
                    vec3 unit_direction = unit_vector(current_ray.direction());
                    auto a = 0.5f * (unit_direction.y() + 1.0f);
                    color sky_color = (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
                    return accumulated_attenuation * sky_color;
                }
            }

        // Exceeded ray bounce limit
        return color(0, 0, 0);
    }


};

#endif