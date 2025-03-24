#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_
#include "edge.h"

namespace cubicat {
    

struct Triangle
{
    Edge    edge0;
    Edge    edge1;
    Edge    edge2;
};

}

#endif