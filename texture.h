#ifndef TEXTURE_H
#define TEXTURE_H

class texture {
    public:
        virtual ~texture() = default;

        virtual color value(float u, float v, const point3& p) const = 0;
};

class solid_color : public texture {
    public:
        solid_color(const color& albedo) : albedo(albedo) {}
        solid_color(float red, float green, float blue): solid_color(color(red,green,blue)) {}

        color value(float u, float v, const point3& p) const override {
            return albedo;
        }
    private:
        color albedo;
};

class checker_texture : public texture {
    public:
        checker_texture(float scale, shared_ptr<texture> even, shared_ptr<texture> odd) : inv_scale(1.0f/scale), even(even), odd(odd) {}
        checker_texture(float scale, const color& c1, const color& c2) : checker_texture(scale, make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

        color value(float u, float v, const point3& p) const override {
            auto xInteger = int(std::floor(inv_scale * p.x()));
            auto yInteger = int(std::floor(inv_scale * p.y()));
            auto zInteger = int(std::floor(inv_scale * p.z()));

            bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

            return isEven ? even->value(u,v,p) : odd->value(u,v,p);
        }

    private:
        float inv_scale;
        shared_ptr<texture> even;
        shared_ptr<texture> odd;
};

#endif