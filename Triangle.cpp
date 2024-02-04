#include "Triangle.h"
#include <iostream>
#include <cmath>

Triangle::Triangle()
{
    shared_material = nullptr;
}

float Triangle::intersect(Ray r)
{
    //Triangle vertices (Capital letters to avoid confusion with Ray r)
    Cartesian3 P = verts[0].Point();
    Cartesian3 Q = verts[1].Point();
    Cartesian3 R = verts[2].Point();

    //Construct vectors
    Cartesian3 u = Q - P;
    Cartesian3 v = R - P;

    //Normalise u -> divide by its norm
    u = u.unit();

    //Compute n -> cross product between u and v and normalise
    Cartesian3 n = u.cross(v).unit();

    //Compute w - > cross product between n and u and normalise
    Cartesian3 w = n.cross(u).unit();

    //Find the value of t
    float t = ((P - r.origin).dot(n) / r.direction.dot(n));

    //Before returning t, we want to make sure it is a valid intersection
    //Calculate point o and convert to PCS
    Cartesian3 o = r.origin +  r.direction*t;
    Cartesian3 oPCS = {(o-P).dot(u), (o-P).dot(w), (o-P).dot(n)};

    //Converting p,q and r to planar coordinates
    Cartesian3 pPCS = {(P-P).dot(u), (P-P).dot(w), (P-P).dot(n)};
    Cartesian3 qPCS = {(Q-P).dot(u), (Q-P).dot(w), (Q-P).dot(n)};
    Cartesian3 rPCS = {(R-P).dot(u), (R-P).dot(w), (R-P).dot(n)};

    //Creating edges to the planar coordiantes
    Cartesian3 rpPCS = rPCS - pPCS;
    Cartesian3 pqPCS = pPCS - qPCS;
    Cartesian3 qrPCS = qPCS - rPCS;

    //We can now perform a 2D half plane test, checking that point o is within pqr
    //Get the normal form of the 2D triangle coordinates
    Cartesian3 rpNorm = {-rpPCS.y, rpPCS.x, 0};
    Cartesian3 pqNorm = {-pqPCS.y, pqPCS.x, 0};
    Cartesian3 qrNorm = {-qrPCS.y, qrPCS.x, 0};

    //Use the half plane test to check that the intersection is within the triangle
    double test1 = rpNorm.dot((oPCS - pPCS));
    double test2 = pqNorm.dot((oPCS - qPCS));
    double test3 = qrNorm.dot((oPCS - rPCS));                                                                                                           ;

    if(test1 <= 0.00002 && test2 <= 0.00002 && test3 <= 0.00002)
    {
        return t;
    }

    //If not, then the point is not inside the triangle
    return -1;

}

Cartesian3 Triangle::barycentric(Cartesian3 o)
{
    //Triangle vertices (Capital letters to avoid confusion with Ray r)
    Cartesian3 P = verts[0].Point();
    Cartesian3 Q = verts[1].Point();
    Cartesian3 R = verts[2].Point();

    //Construct vectors
    Cartesian3 u = Q - P;
    Cartesian3 v = R - P;

    //Normalise u -> divide by its norm
    u = u.unit();

    //Compute n -> cross product between u and v and normalise
    Cartesian3 n = u.cross(v).unit();

    //Compute w - > cross product between n and u and normalise
    Cartesian3 w = n.cross(u).unit();

    //Converting o, p, q and r to planar coordinates
    Cartesian3 oPCS = {(o-P).dot(u), (o-P).dot(w), (o-P).dot(n)};
    Cartesian3 pPCS = {(P-P).dot(u), (P-P).dot(w), (P-P).dot(n)};
    Cartesian3 qPCS = {(Q-P).dot(u), (Q-P).dot(w), (Q-P).dot(n)};
    Cartesian3 rPCS = {(R-P).dot(u), (R-P).dot(w), (R-P).dot(n)};

    //Create edges and form normals from them
    Cartesian3 rp = rPCS - pPCS;
    Cartesian3 qr = qPCS - rPCS;
    Cartesian3 pq = pPCS - qPCS;

    Cartesian3 rpNorm = {-rp.y, rp.x, 0};
    Cartesian3 qrNorm = {-qr.y, qr.x, 0};
    Cartesian3 pqNorm = {-pq.y, pq.x, 0};

    //Calculate distances
    float distOtoRP, distOtoQR, distOtoPQ;
    float distQtoRP, distPtoQR, distRtoPQ;

    //Calculate the distances from each edge to o
    distOtoRP = (rpNorm.dot(pPCS - oPCS)) / rpNorm.length();
    distOtoQR = (qrNorm.dot(rPCS - oPCS)) / qrNorm.length();
    distOtoPQ = (pqNorm.dot(qPCS - oPCS)) / pqNorm.length();

    //Calculate the distance between each vertex and their opposite edge
    distPtoQR = (qrNorm.dot(rp)) / qrNorm.length();
    distQtoRP = (rpNorm.dot(pq)) / rpNorm.length();
    distRtoPQ = (pqNorm.dot(qr)) / pqNorm.length();

    //Calculate barycentric coordinates
    float alpha = distOtoQR / distPtoQR;
    float beta = distOtoRP / distQtoRP;
    float gamma = distOtoPQ / distRtoPQ;

    Cartesian3 bc;
    bc.x = alpha;
    bc.y = beta;
    bc.z = gamma;

    return bc;
}

Homogeneous4 Triangle::calculatePhong(Homogeneous4 lightPosition, Homogeneous4 lightColour, Cartesian3 barycentricCoords, bool inShadow)
{
    //Set up the point p
    Cartesian3 P = verts[0].Point();
    Cartesian3 Q = verts[1].Point();
    Cartesian3 R = verts[2].Point();

    //Get p's position by using barycentric coordinates
    Cartesian3 p = (P * barycentricCoords.x) + (Q * barycentricCoords.y) + (R * barycentricCoords.z);
    Cartesian3 normal = (normals[0].Vector()* barycentricCoords.x) + (normals[1].Vector() * barycentricCoords.y) + (normals[2].Vector() * barycentricCoords.z);
    normal = normal.unit();
    //We know lightPosition, and eye = (0,0,0)
    Cartesian3 eye = {0,0,0};

    //Set up vectors to the point from the light source, and from the eye
    //Don't normalise vl yet since we want its distance for quadratic attenuation calculation
    Cartesian3 vl = (lightPosition.Point() - p);
    Cartesian3 ve = (eye - p).unit();

    //Quadratic Attenuation - 1 / d^2 (where d is the distance from the light to the point)
    float vlDistance = vl.length();
    float attenuation = 1.0 / (1+ vlDistance * vlDistance);


    Cartesian3 lColour = lightColour.Vector();
    Homogeneous4 phong;


    //Emissive is just a constant
    Cartesian3 emissive = shared_material->emissive;

    //Ambient is uniform -> the same in all directions
    Cartesian3 ambient = {lColour.x * shared_material->ambient.x * attenuation,
                          lColour.y * shared_material->ambient.y * attenuation,
                          lColour.z * shared_material->ambient.z * attenuation};

    //Specular lighting
    //Based on the angle between the normal and the bisector

    //Given vl and ve, calculate normal and bisector
    vl = vl.unit();
    //Cartesian3 normal = vl.cross(ve).unit();
    Cartesian3 bisector = (vl + ve).unit();

    Cartesian3 specular = {0,0,0};
    Cartesian3 diffuse = {0,0,0};

    //Test whether the light can see the surface
    if (normal.dot(vl) > 0)
    {
        float ndotV =std::pow(normal.dot(bisector), shared_material->shininess);
        specular = {lColour.x * shared_material->specular.x * ndotV * attenuation,
                    lColour.y * shared_material->specular.y * ndotV * attenuation,
                    lColour.z * shared_material->specular.z * ndotV * attenuation
                    };

        float ndotbisector = normal.dot(vl);
        diffuse = {lColour.x * shared_material->diffuse.x * ndotbisector * attenuation,
                   lColour.y * shared_material->diffuse.y * ndotbisector * attenuation,
                   lColour.z * shared_material->diffuse.z * ndotbisector * attenuation};

    }

    //If the point is in shadow, then we set specular and diffuse to 0
    if (inShadow)
    {
        specular = {0,0,0};
        diffuse = {0,0,0};

    }


    //Compute the total light for the point
    Cartesian3 total = specular + diffuse + ambient + emissive;
    //Cartesian3 total = diffuse;

    phong = Homogeneous4(total);

    return phong;
}

