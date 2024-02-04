#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Homogeneous4.h"
#include "Material.h"
#include "Ray.h"

class Triangle
{
public:
    Homogeneous4 verts[3];
    Homogeneous4 normals[3];
    Homogeneous4 colors[3];
    Cartesian3 uvs[3];

    Material *shared_material;
    Triangle();

    float intersect(Ray r);
    Cartesian3 barycentric(Cartesian3 o);

    Homogeneous4 calculatePhong(Homogeneous4 lightPosition, Homogeneous4 lightColour, Cartesian3 barycentricCoords, bool inShadow);
};

#endif // TRIANGLE_H
