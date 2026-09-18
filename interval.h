#ifndef INTERVAL_H
#define INTERVAL_H

class interval {
    public:
        float min, max;

        interval() : min(+infinity_f), max(-infinity_f) {}

        interval(float min, float max) : min(min), max(max) {}

        interval(const interval& a, const interval& b){
            min = a.min <= b.min ? a.min : b.min;
            max = a.max >= b.max ? a.max : b.max;
        }

        float size() const {
            return max - min;
        }

        bool contains(float x) const {
            return min <= x && x <= max;
        }

        bool surrounds(float x) const {
            return min < x && x < max;
        }

        float clamp(float x) const {
            if(x < min) return min;
            if(x > max) return max;
            return x;
        }

        interval expand(float delta) const {
            auto padding = delta/2;
            return interval(min - padding, max + padding);
        }

        static const interval empty, universe;
};

const interval interval::empty = interval(+infinity_f, -infinity_f);
const interval interval::universe = interval(-infinity_f, +infinity_f);

#endif