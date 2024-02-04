#include "Scene.h"

Scene::Scene(std::vector<ThreeDModel> *texobjs, RenderParameters *renderp)
{
    objects = texobjs;
    rp = renderp;
    Cartesian3 ambient = Cartesian3(0.5f, 0.5f, 0.5f);
    Cartesian3 diffuse = Cartesian3(0.5f, 0.5f, 0.5f);
    Cartesian3 specular = Cartesian3(0.5f, 0.5f, 0.5f);
    Cartesian3 emissive = Cartesian3(0,0,0);

    float shininess = 1.0f;
    default_mat = new Material(ambient, diffuse, specular, emissive, shininess);
}

void Scene::updateScene()
{
    triangles.clear();
    for (int i = 0; i < int(objects ->size()); i++)
    {
        typedef unsigned int uint;
        ThreeDModel obj = objects->at(uint(i));
        for (uint face = 0; face < obj.faceVertices.size(); face++)
        {
            for (uint triangle = 0; triangle < obj.faceVertices[face].size()-2; triangle++)
            {
                Triangle t;
                for (uint vertex = 0; vertex < 3; vertex++)
                {
                    uint faceVertex = 0;
                    if (vertex != 0)
                        faceVertex = triangle + vertex;

                    Homogeneous4 v = Homogeneous4(obj.vertices[obj.faceVertices     [face][faceVertex]].x,
                                                  obj.vertices[obj.faceVertices     [face][faceVertex]].y,
                                                  obj.vertices[obj.faceVertices     [face][faceVertex]].z);

                    v = getModelView() * v;

                    t.verts[vertex] = v;
                    Homogeneous4 n = Homogeneous4(obj.normals[obj.faceNormals       [face][faceVertex]].x,
                                                  obj.normals[obj.faceNormals       [face][faceVertex]].y,
                                                  obj.normals[obj.faceNormals       [face][faceVertex]].z,
                                                  0.0f);

                    n = getModelView() * n;
                    t.normals[vertex] = n;

                    Cartesian3 tex = Cartesian3(obj.textureCoords[obj.faceTexCoords[face][faceVertex]].x,
                                                obj.textureCoords[obj.faceTexCoords[face][faceVertex]].y,
                                                0.0f);
                    t.uvs[vertex] = tex;
                    t.colors[vertex] = Cartesian3(0.7f, 0.7f, 0.7f);
                }

                if(obj.material == nullptr)
                {
                    t.shared_material = default_mat;
                }

                else
                {
                    t.shared_material = obj.material;
                }

                triangles.push_back(t);
            }
        }
    }
}

Matrix4 Scene::getModelView()
{
    //Initialise result matrix
    Matrix4 result;

    //Grab all of the necessary matrices to build the model view
    Matrix4 rotation, translation;

    //Rotation
    rotation = rp->rotationMatrix;

    //Translation
    translation.SetTranslation({rp->xTranslate, rp->yTranslate, rp->zTranslate-1});

    result = translation * rotation;
    return result;
}

Scene::CollisionInfo Scene::closestTriangle(Ray r)
{
    Scene::CollisionInfo ci;

    //Set a placeholder value so there isn't an out of bounds error
    ci.t = -1;

    //We loop through every triangle in the scene
    for (Triangle currentTriangle : triangles)
    {
        //Get the value of t by intersection test
        float t = currentTriangle.intersect(r);

        //Test if the current value of t is less than the currently stored value of t (means that the triangle is closer to the ray)
        //First of all, test that the placeholder value of ci.t still holds
        if (ci.t < 0)
        {
            //Replace the placeholder value with current t, if it's positive
            if(t > 0)
            {
                ci.tri = currentTriangle;
                ci.t = t;
            }
        }

        //When ci.t has a positive value, check if current t is less than saved t
        else
        {
            if(t > 0 && t < ci.t)
            {
                //std::cout<< "Updated t -> " << t << std::endl;
                ci.tri = currentTriangle;
                ci.t = t;
            }
        }
    }

    return ci;
}
