#ifndef _EDGE_H_
#define _EDGE_H_

namespace cubicat {


struct Edge {
    float x;    // x pos at yMin
    float dx;   // delta x equal to 1 / slope
    float yMin; // 
    float yMax; // 
    float u;    // u at yMin (divided by w for 3D)
    float v;    // v at yMin (divided by w  for 3D)
    float du;   // delta u (divided by w  for 3D)
    float dv;   // delta v (divided by w  for 3D)
    float illum; // 
    float dillum; // delta illum
    float z;        // z depth
    float dz;        // delta z
    float w;        // homogeneous term
    float dw;
    inline void invalidate() {
        yMin = 99999;
        yMax = -99999;
    }
};

}

#endif