#ifndef QUAD_H
#define QUAD_H

#include "hittable.hpp"

class quad : public hittable{
  public :
    quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
    :Q(Q), u(u), v(v), mat(mat)
    {
        set_bounding_box();
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n,n);
    }

    virtual void set_bounding_box(){
        // compute the bounding box of all four vertices
        // for aabb searching 
        auto bbox_diagonal1 = aabb(Q, Q + u + v);
        auto bbox_diagonal2 = aabb(Q + u , Q  + v);
        bbox = aabb(bbox_diagonal1, bbox_diagonal2);
    }

    aabb bounding_box() const override {return bbox;}
    bool hit (const ray&r, interval ray_t, hit_record& rec)const override{
        auto denom = dot(normal, r.direction());
        // No hit if the ray is parallel to the plane.
        if (std::fabs(denom) < 1e-8)
            return false;

        // Return false if the hit point parameter t is outside the ray interval.
        auto t = (D - dot(normal, r.origin())) / denom ;
        if (!ray_t.contains(t)) 
            return false;

        // Determine if the hit point lies within the planar shape using its plane coordinates.
        auto intersection = r.at(t);

        vec3 v_qp = intersection - Q; 
        // alpha is scaler u 
        auto alpha = dot(w, cross(v_qp,v));
        // beta is scaler v ;
        auto beta  = dot(w, cross(u, v_qp));

        if (!is_interior(alpha, beta, rec))
            return false;

        rec.t = t ;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);
        return true;
    }

    bool is_interior(double a, double b, hit_record& rec) const {
        interval unit_interval = interval(0, 1);
        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }
  private :
    point3 Q; 
    vec3 u ;
    vec3 v;
    vec3 w;
    shared_ptr<material> mat;
    aabb bbox;
    vec3 normal;
    double D;


};



#endif

