#include "Ray.h"

Ray::Ray(Cartesian3 og, Cartesian3 dir)
{
    origin = og;
    direction = dir;
}

Ray::Ray()
{
    origin = {0,0,0};
    direction = {0,0,0};
}
