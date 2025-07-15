#ifndef UTILIS_H
#define UTILIS_H

#include <cmath>
#include <random>
#include <iostream>
#include <limits>
#include <memory>
#include "interval.hpp"

// C++ Std Usings

using std::make_shared;
using std::shared_ptr;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    //deg -> 角度 
    //radi -> 徑度
    return degrees * pi / 180.0;
}

inline double random_double (){
    // Returns a random real in [0,1).
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double();
}

inline double random_double_mt() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}


// Common Headers

#include "color.hpp"
#include "ray.hpp"
#include "vec3.hpp"

#endif