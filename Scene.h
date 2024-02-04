#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "ThreeDModel.h"
#include "RenderParameters.h"
#include "Triangle.h"
#include "Material.h"
#include "Ray.h"

class Scene
{
public:
    std::vector<ThreeDModel>* objects;
    RenderParameters* rp;
    std::vector<Triangle> triangles;
    Scene(std::vector<ThreeDModel> *texobjs, RenderParameters *renderp);
    void updateScene();
    Material *default_mat;

    Matrix4 getModelView();

    struct CollisionInfo
    {
        Triangle tri;
        float t;
    };

    CollisionInfo closestTriangle (Ray r);
};

#endif // SCENE_H
