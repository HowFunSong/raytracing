#ifndef SPHERE_H
#define spHERE_H

#include "vec3.hpp"
#include "hittable.hpp"

class sphere : public hittable {
  public:
    sphere(const point3& center, double radius, shared_ptr<material> mat) 
    : center(center), radius(std::fmax(0,radius)), mat(mat) {
        // TODO: Initialize the material pointer `mat`.
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto b = -2 * dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = b*b - 4*a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (- b - sqrtd) / (2*a);
        if (!ray_t.surround(root)) {
            root = (- b + sqrtd) / (2*a);
            if (!ray_t.surround(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        return true;
    }

  private:
    point3 center;
    double radius;
    shared_ptr<material> mat;
};
#endif