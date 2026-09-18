#ifndef SCENES_H
#define SCENES_H

#include "hittable_list.h"

class scenes{
    public:

        hittable_list RT_Weekend(){
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
            return world;
        }

        hittable_list perlin_scene(){
            hittable_list world;

            auto pertext = make_shared<noise_texture>(4);
            world.add(make_shared<sphere>(point3(0,-1000,0),1000,make_shared<lambertian>(pertext)));
            world.add(make_shared<sphere>(point3(0,2,0),2,make_shared<lambertian>(pertext)));

            world = hittable_list(make_shared<bvh_node>(world));
            return world;
        }

};


#endif