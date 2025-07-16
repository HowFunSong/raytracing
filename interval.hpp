#ifndef INTERVAL_H
#define INTERVAL_H

#include <limits>

class interval{
  public :
    double min, max;
    interval(): min(-std::numeric_limits<double>::infinity()), max(+std::numeric_limits<double>::infinity()){};
    interval(double min, double max) : min(min), max(max) {}
    interval(const interval&a, const interval&b){
        // Create the interval tightly enclosing the two input intervals.
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }
    
    double size() const{
        return max - min;
    }

    bool contains(double x ) const{
        return min <= x && x <= max ;
    }

    bool surround (double x) const {
        return min < x && x < max ;
    }

    double clamp(double x)const{
        if (x < min ) return min;
        if (x > max ) return max;
        return x; 
    }

    interval expand(double delta) const{
        auto padding = delta / 2;
        return interval (min - padding , max + padding);
    }
    
    
    static const interval empty, universe;


    ~interval() = default;
    
};

const interval interval::empty = interval(+std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity());
const interval interval::universe = interval(-std::numeric_limits<double>::infinity(), +std::numeric_limits<double>::infinity());
#endif