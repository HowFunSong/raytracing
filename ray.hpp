#ifndef RAY_H
#define RAY_H

#include "vec3.hpp"

class ray{
public :
    ray(){};

    ray(const point3& p, const vec3& v) 
        : ray(p, v, 0){} ;

    ray(const point3& p, const vec3& v, double time) 
        : orig(p), dir(v), tm(time){} ;

    const point3 origin() const {return orig;}
    const vec3 direction() const {return dir;}

    point3 at (double t )const {return orig + t * dir ;}

    double time () const {return tm ;}
        
private :
    point3 orig;
    vec3 dir ;
    double tm ;
};

#endif 