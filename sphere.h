#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable{
    public:
        sphere(const point3& static_center, float radius, shared_ptr<material> mat) 
        : center(static_center), radius(std::fmax(0,radius)), mat(mat)
        {
            auto rvec = vec3(radius, radius, radius);
            bbox = aabb(static_center - rvec, static_center + rvec);
        }

        bool hit(const ray& r, interval ray_t, hit_record& rec) const override{
            vec3 oc = center - r.origin();
            auto a = r.direction().length_squared();
            auto h = dot(r.direction(), oc);
            auto c = oc.length_squared() - radius * radius;

            auto discriminant = h*h - a*c;
            if (discriminant < 0)
                return false;
            
            auto sqrtd = std::sqrt(discriminant);

            // find the nearest root that lies in the acceptable range
            auto root = (h-sqrtd) / a;
            if (!ray_t.surrounds(root)){
                root = (h + sqrtd) / a;
                if (!ray_t.surrounds(root)){
                    return false;
                }
            }

            rec.t = root;
            rec.p = r.at(rec.t);
            vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.mat = mat;

            return true;
        }

        aabb bounding_box() const override {return bbox;}
    private:
        point3 center;
        float radius;
        shared_ptr<material> mat;
        aabb bbox;
};

#endif