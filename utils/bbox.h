#ifndef BBOX_H
#define BBOX_H

#include <iostream>
#include <limits.h>
#include <float.h>
#include <stdlib.h>

template <class T>
class Bbox {
    T x_min, x_max, y_min, y_max;
public:
    Bbox(){
        x_min = x_max =  y_min = y_max = 0;
    }

    Bbox(T xmin, T xmax, T ymin, T ymax){
        x_min = xmin;
        y_max = ymax;
        x_max = xmax;
        y_min = ymin;
    }
    
    T width() const {
    	T w = x_max - x_min;
        return w > 0 ? w : 0;
    }
    
    T height()  const {
    	T h = y_max - y_min;
        return h > 0 ? h : 0;
    }
    
    void width(T w){
    	x_max = x_min + w;
    }

    void height(T h){
    	y_max = y_min + h;
    }

    T xmin() const {return x_min;}
    T xmax() const {return x_max;}
    T ymin() const {return y_min;}
    T ymax() const {return y_max;}
    
    void xmax(T v){x_max = v;}
    void xmin(T v){x_min = v;}
    void ymax(T v){y_max = v;}
    void ymin(T v){y_min = v;}

    bool intersects(const Bbox& b) const {
        return !( b.x_min > x_max || b.x_max < x_min || b.y_max < y_min || b.y_min > y_max );
    }
    bool contains(T x, T y) const {
        return !(x < x_min || x > x_max || y < y_min || y > y_max);
    }
    
    void extend(T x, T y){
        if( x < x_min ) x_min = x;
        if( x > x_max ) x_max = x;
        if( y < y_min ) y_min = y;
        if( y > y_max ) y_max = y;
    }

    void extend(const Bbox& b){
    	extend(b.x_min, b.y_min);
    	extend(b.x_max, b.y_max);
    }

    void move_to(T x, T y){
    	T w = x_max - x_min;
    	T h = y_max - y_min;
    	x_min = x;
    	y_min = y;
    	x_max = x + w;
    	y_max = y + h;
    }

    void crop(const Bbox& b){
    	if (x_max > b.x_max) x_max = b.x_max;
    	if (y_max > b.y_max) y_max = b.y_max;
    	if (x_min < b.x_min) x_min = b.x_min;
    	if (y_min < b.y_min) y_min = b.y_min;
    }
};

typedef Bbox<int>   IBbox;
typedef Bbox<float> FBbox;

inline void
empty_bbox(IBbox& b){
	b.xmin(INT_MAX);
	b.xmax(INT_MIN);
	b.ymin(INT_MAX);
	b.ymax(INT_MIN);
}

inline void
empty_bbox(FBbox& b){
	b.xmin(FLT_MAX);
	b.xmax(FLT_MIN);
	b.ymin(FLT_MAX);
	b.ymax(FLT_MIN);
}

std::ostream& operator << (std::ostream & out, IBbox const& data);
std::ostream& operator << (std::ostream & out, FBbox const& data);

#endif
