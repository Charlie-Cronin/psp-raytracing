#ifndef SCENES_H
#define SCENES_H

#include "hittable_list.h"
#include "quad.h"

class scenes{
    public:
        int IMAGE_WIDTH;
        int IMAGE_HEIGHT;
        int BUFFER_WIDTH;
        int SAMPLES;
        int MAX_DEPTH;

        scenes(int IMAGE_WIDTH, int IMAGE_HEIGHT, int BUFFER_WIDTH, int SAMPLES, int MAX_DEPTH) : IMAGE_WIDTH(IMAGE_WIDTH), IMAGE_HEIGHT(IMAGE_HEIGHT), BUFFER_WIDTH(BUFFER_WIDTH), SAMPLES(SAMPLES), MAX_DEPTH(MAX_DEPTH) {}


        hittable_list RT_Weekend(camera& cam){
            hittable_list world;

            auto checker = make_shared<checker_texture>(0.32f, color(0.2f, 0.3f, 0.1f), color(0.9f, 0.9f, 0.9f));
            world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

            

            for (int a = -3; a < 3; a++){
                for (int b = -3; b < 3; b++){
                    auto choose_mat = random_float();
                    point3 centre(a+0.9f*random_float(), 0.2,b + 0.9*random_float());

                    if ((centre - point3(4,0.2f,0)).length() > 0.9f){
                        shared_ptr<material> sphere_material;

                        if (choose_mat < 0.0f){
                            auto albedo = color::random() * color::random();
                            sphere_material = make_shared<lambertian>(albedo);
                            world.add(make_shared<sphere>(centre,0.2f,sphere_material));
                        }
                        else if (choose_mat < 0.95f){
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


            world = hittable_list(make_shared<bvh_node>(world));


            // CAMERA SETTINGS
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

            return world;
        }

        hittable_list perlin_scene(camera& cam){
            hittable_list world;

            auto pertext = make_shared<noise_texture>(4);
            world.add(make_shared<sphere>(point3(0,-1000,0),1000,make_shared<lambertian>(pertext)));
            world.add(make_shared<sphere>(point3(0,2,0),2,make_shared<lambertian>(pertext)));

            world = hittable_list(make_shared<bvh_node>(world));

            // CAMERA SETTINGS
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

            return world;
        }

        hittable_list quads(camera& cam){
            hittable_list world;

            auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));
            auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));
            auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));
            auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
            auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));

            world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0,0,-4), vec3(0,4, 0), left_red));
            world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4,0, 0), vec3(0,4, 0), back_green));
            world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0,0, 4), vec3(0,4, 0), right_blue));
            world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4,0, 0), vec3(0,0, 4), upper_orange));
            world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4,0, 0), vec3(0,0,-4), lower_teal));

            world = hittable_list(make_shared<bvh_node>(world));

            // CAMERA SETTINGS
            cam.image_width = IMAGE_WIDTH;
            cam.image_height = IMAGE_HEIGHT;
            cam.buffer_width = BUFFER_WIDTH;
            cam.samples_per_pixel = SAMPLES;
            cam.max_depth = MAX_DEPTH;

            cam.vfov = 80;
            cam.lookfrom = point3(0,0,9);
            cam.lookat = point3(0,0,0);

            cam.defocus_angle = 0.6f;
            cam.focus_dist = 10.0f;

            return world;
        }

};


#endif