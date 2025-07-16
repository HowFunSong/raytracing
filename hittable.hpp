#ifndef HITTABLE_H
#define HITTABLE_H

#include "utilis.hpp"
#include "aabb.hpp"

class material; 

class hit_record{
  public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal){
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.
        front_face = dot( r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
        
    }
};

// hittable is the base class of sphere
class hittable{
  public:
    virtual ~hittable() = default;
    virtual bool hit(const ray&r, interval ray_t, hit_record& rec) const = 0;
    virtual aabb bounding_box() const = 0;


  private:

};

#endif