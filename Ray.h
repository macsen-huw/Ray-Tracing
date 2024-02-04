#ifndef RAY_H
#define RAY_H

#include "Cartesian3.h"

class Ray
{
public:
    Ray(Cartesian3 og, Cartesian3 dir);
    Ray();
    Cartesian3 origin;
    Cartesian3 direction;
};

#endif // RAY_H
