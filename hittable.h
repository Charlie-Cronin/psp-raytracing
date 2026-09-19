#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "aabb.h"

class material;

class hit_record{
    public:
        point3 p;
        vec3 normal;
        shared_ptr<material> mat;
        float t; 
        float u;
        float v;
        bool front_face;

        void set_face_normal (const ray& r, const vec3& outward_normal){
            // sets the hit record normal vector
            // NOTE: parameter 'outward_normal' is assumed to have unit length 

            front_face = dot(r.direction(), outward_normal) < 0;
            normal = front_face ? outward_normal : -outward_normal;
        }
};


class hittable {
    public:
        virtual ~hittable() = default;

        virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;

        virtual aabb bounding_box() const = 0;
};

class translate : public hittable {
    public:
        translate(shared_ptr<hittable> object, const vec3& offset) : object(object), offset(offset){
            bbox = object->bounding_box() + offset;
        }

        bool hit( const ray& r, interval ray_t, hit_record& rec) const override{
            ray offset_r(r.origin() - offset, r.direction());

            if(!object->hit(offset_r, ray_t, rec))
                return false;
            
            rec.p += offset;

            return true;
        }

        aabb bounding_box() const override {return bbox;}

    private:
        shared_ptr<hittable> object;
        vec3 offset;
        aabb bbox;
};

class rotate_y : public hittable {
    public: 
        rotate_y(shared_ptr<hittable> object, float angle) : object(object){
            float radians = degrees_to_radians(angle);
            sin_theta = std::sin(radians);
            cos_theta = std::cos(radians);
            bbox = object->bounding_box();

            point3 min(infinity_f, infinity_f, infinity_f);
            point3 max(-infinity_f, -infinity_f, -infinity_f);

            for (int i = 0; i < 2; i++){
                for (int j = 0; j < 2; j++){
                    for (int k = 0; k < 2; k++){
                        auto x = i*bbox.x.max + (1-i)*bbox.x.min;
                        auto y = j*bbox.y.max + (1-j)*bbox.y.min;
                        auto z = k*bbox.z.max + (1-k)*bbox.z.min;

                        auto newx =  cos_theta * x + sin_theta*z;
                        auto newz = -sin_theta * x + cos_theta*z;

                        vec3 tester(newx, y, newz);

                        for (int c = 0; c < 3; c++) {
                            min[c] = std::fmin(min[c], tester[c]);
                            max[c] = std::fmax(max[c], tester[c]);
                        }
                    }
                }
            }

            bbox = aabb(min, max);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override{
            // transform world space to object space
            auto origin = point3(
                (cos_theta * r.origin().x()) - (sin_theta * r.origin().z()),
                r.origin().y(),
                (sin_theta * r.origin().x()) + (cos_theta * r.origin().z())
            );

            auto direction = vec3(
                (cos_theta * r.direction().x()) - (sin_theta * r.direction().z()),
                r.direction().y(),
                (sin_theta * r.direction().x()) + (cos_theta * r.direction().z())
            );

            ray rotated_r(origin, direction);

            if(!object->hit(rotated_r, ray_t, rec))
                return false;

            rec.p = point3(
                (cos_theta * rec.p.x()) + (sin_theta * rec.p.z()),
                rec.p.y(),
                (-sin_theta * rec.p.x()) + (cos_theta * rec.p.z())
            );
            
            rec.normal = vec3(
                (cos_theta * rec.normal.x()) + (sin_theta * rec.normal.z()),
                rec.normal.y(),
                (-sin_theta * rec.normal.x()) + (cos_theta * rec.normal.z())
            );

            return true;
        }

        aabb bounding_box() const override { return bbox; }
    
    private:
        shared_ptr<hittable> object;
        float sin_theta;
        float cos_theta;
        aabb bbox;
};

#endif