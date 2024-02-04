//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////


#include <math.h>
#include <random>
#include <QTimer>
#include <iostream>
// include the header file
#include "RaytraceRenderWidget.h"

#define N_THREADS 16
#define N_LOOPS 100
#define N_BOUNCES 5
#define TERMINATION_FACTOR 0.35f

// constructor
RaytraceRenderWidget::RaytraceRenderWidget
        (   
        // the geometric object to show
        std::vector<ThreeDModel>      *newTexturedObject,
        // the render parameters to use
        RenderParameters    *newRenderParameters,
        // parent widget in visual hierarchy
        QWidget             *parent
        )
    // the : indicates variable instantiation rather than arbitrary code
    // it is considered good style to use it where possible
    : 
    // start by calling inherited constructor with parent widget's pointer
    QOpenGLWidget(parent),
    // then store the pointers that were passed in
    texturedObjects(newTexturedObject),
    renderParameters(newRenderParameters)
    { // constructor

    scene = new Scene(texturedObjects, renderParameters);
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &RaytraceRenderWidget::forceRepaint);
    timer->start(30);

    //scene = Scene(texturedObjects, renderParameters);


    } // constructor


// destructor
RaytraceRenderWidget::~RaytraceRenderWidget()
    { // destructor
    // empty (for now)
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
    // and OpenGL cleanup is taken care of by Qt
    } // destructor                                                                 

// mouse-handling
void RaytraceRenderWidget::mousePressEvent(QMouseEvent *event)
    { // RaytraceRenderWidget::mousePressEvent()
    // store the button for future reference
    int whichButton = int(event->button());
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;

    
    // and we want to force mouse buttons to allow shift-click to be the same as right-click
    unsigned int modifiers = event->modifiers();
    
    // shift-click (any) counts as right click
    if (modifiers & Qt::ShiftModifier)
        whichButton = Qt::RightButton;
    
    // send signal to the controller for detailed processing
    emit BeginScaledDrag(whichButton, x,y);
    } // RaytraceRenderWidget::mousePressEvent()
    
void RaytraceRenderWidget::mouseMoveEvent(QMouseEvent *event)
    { // RaytraceRenderWidget::mouseMoveEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit ContinueScaledDrag(x,y);
    } // RaytraceRenderWidget::mouseMoveEvent()
    
void RaytraceRenderWidget::mouseReleaseEvent(QMouseEvent *event)
    { // RaytraceRenderWidget::mouseReleaseEvent()
    // scale the event to the nominal unit sphere in the widget:
    // find the minimum of height & width   
    float size = (width() > height()) ? height() : width();
    // scale both coordinates from that
    float x = (2.0f * event->x() - size) / size;
    float y = (size - 2.0f * event->y() ) / size;
    
    // send signal to the controller for detailed processing
    emit EndScaledDrag(x,y);
    } // RaytraceRenderWidget::mouseReleaseEvent()

// called when OpenGL context is set up
void RaytraceRenderWidget::initializeGL()
    { // RaytraceRenderWidget::initializeGL()
	// this should remain empty
    } // RaytraceRenderWidget::initializeGL()

// called every time the widget is resized
void RaytraceRenderWidget::resizeGL(int w, int h)
    { // RaytraceRenderWidget::resizeGL()
    // resize the render image
    frameBuffer.Resize(w, h);
    } // RaytraceRenderWidget::resizeGL()
    
// called every time the widget needs painting
void RaytraceRenderWidget::paintGL()
    { // RaytraceRenderWidget::paintGL()
    // set background colour to white
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // and display the image
    glDrawPixels(frameBuffer.width, frameBuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.block);
    } // RaytraceRenderWidget::paintGL()

void RaytraceRenderWidget::Raytrace()
{
    scene->updateScene();
    raytracingThread = std::thread(&RaytraceRenderWidget::RaytraceThread, this);
    raytracingThread.detach();
}

void RaytraceRenderWidget::RaytraceThread()
{
    frameBuffer.clear(RGBAValue(0.0f, 0.0f, 0.0f, 1.0f));

#pragma omp parallel for schedule(dynamic)
    for(int j = 0; j < frameBuffer.height; j++)
    {
        for (int i = 0; i < frameBuffer.width; i++)
        {
            Homogeneous4 color;
            Ray ray = calculateRay(i, j, !renderParameters->orthoProjection);

            if (renderParameters->reflectionEnabled)
            {
                color = calculateLightforRay(ray, N_BOUNCES);
            }

            else
            {
                Scene::CollisionInfo hitInfo = scene->closestTriangle(ray);
                if(hitInfo.t > 0)
                {
                    color = {1.0f, 1.0f, 1.0f};
                    //Calculate barycentric coordinates
                     //We calculate o from our t, since o = origin + t*direction
                    Cartesian3 o = ray.origin + (hitInfo.t*ray.direction);
                    Cartesian3 barycentricCoords = hitInfo.tri.barycentric(o);

                    if (renderParameters->interpolationRendering)
                    {
                        //Perform barycentric interpolation if enabled
                        color = (hitInfo.tri.normals[0] * barycentricCoords.x) + (hitInfo.tri.normals[1] * barycentricCoords.y) + (hitInfo.tri.normals[2] * barycentricCoords.z);
                        color.x = abs(color.x);
                        color.y = abs(color.y);
                        color.z = abs(color.z);

                    }

                    if (renderParameters->phongEnabled)
                    {
                        Homogeneous4 finalColour;
                        //Loop through every light, and calculate the Phong lighting
                        for (unsigned int i = 0; i < renderParameters->lights.size(); i++)
                        {
                                //Apply modelView matrix to the light position so it's in the right place
                                Homogeneous4 lightPosition = scene->getModelView() * renderParameters->lights[i]->GetPositionCenter();
                                Homogeneous4 lightColour = renderParameters->lights[i]->GetColor();

                                Homogeneous4 phong = hitInfo.tri.calculatePhong(lightPosition, lightColour, barycentricCoords, false);

                                finalColour = finalColour + phong;

                        }
                        //Set the colour to be the colour calculated using Blinn-Phong
                        color = finalColour;

                     }

                    if(renderParameters->shadowsEnabled)
                    {
                        Homogeneous4 finalColour;
                        //Loop through every light, and calculate the Phong lighting
                        for (unsigned int i = 0; i < renderParameters->lights.size(); i++)
                        {
                                //Apply modelView matrix to the light position so it's in the right place
                                Homogeneous4 lightPosition = scene->getModelView() * renderParameters->lights[i]->GetPositionCenter();
                                Homogeneous4 lightColour = renderParameters->lights[i]->GetColor();

                                //Determines if a point is in Shadow
                                bool inShadow = false;

                                //Experimenting with normals
                                Homogeneous4 pNormal = hitInfo.tri.normals[0];
                                Homogeneous4 qNormal = hitInfo.tri.normals[1];
                                Homogeneous4 rNormal = hitInfo.tri.normals[2];

                                Homogeneous4 normal = (pNormal * barycentricCoords.x) + (qNormal * barycentricCoords.y) + (rNormal * barycentricCoords.z);

                                //Adjust the starting position of secondary Ray to prevent the ray intersecting with itself and causing shadow acne
                                float epsilon = 0.001;
                                Cartesian3 secondaryRayOrigin = o + (epsilon * Cartesian3 (normal.x, normal.y, normal.z));

                                //Initialise the direction of the secondary ray (from the intersection point o to the light position)
                                Cartesian3 secondaryRayDirection = (lightPosition.Point() - secondaryRayOrigin).unit();



                                //Initialise secondary Ray
                                Ray secondaryRay = Ray(secondaryRayOrigin, secondaryRayDirection);
                                //Calculate closest intersection to the secondary ray
                                Scene::CollisionInfo secondaryHitInfo = scene->closestTriangle(secondaryRay);

                                if(secondaryHitInfo.t > 0)
                                {
                                    //Get the distances of the ray origin to the intersected triangle, and the light
                                    double lengthToTriangle = (secondaryHitInfo.tri.verts->Point() - secondaryRayOrigin).length();
                                    double lengthToLight = (lightPosition.Point() - secondaryRayOrigin).length();

                                    //If an object is closer to the ray than the light, then the point o is in shadow
                                    if((lengthToTriangle < lengthToLight) && !(secondaryHitInfo.tri.shared_material->isLight()))
                                    {
                                        inShadow = true;
                                    }

                                }


                                Homogeneous4 phong = hitInfo.tri.calculatePhong(lightPosition, lightColour, barycentricCoords, inShadow);
                                finalColour = finalColour + phong;

                        }

                        //Set the colour to be the colour calculated using Blinn-Phong
                        color = finalColour;

                    }


                }

                else
                {
                    color = {i/float(frameBuffer.height), j/float(frameBuffer.width), 0};

                }

            }

            //Gamma correction
            float gamma = 2.2f;
            color.x = pow(color.x, 1/gamma);
            color.y = pow(color.y, 1/gamma);
            color.z = pow(color.z, 1/gamma);
            frameBuffer[j][i] = RGBAValue(color.x*255.0f,
                                          color.y*255.0f,
                                          color.z*255.0f,
                                          255.0f);

        }
    }
}

Homogeneous4 RaytraceRenderWidget::calculateLightforRay(Ray ray, int depth)
{
    Homogeneous4 colour;
    Scene::CollisionInfo hitInfo = scene->closestTriangle(ray);

    if (hitInfo.t > 0)
    {
        //Calculate barycentric coordinates
        //We calculate o from our t, since o = origin + t*direction
        Cartesian3 o = ray.origin + (hitInfo.t*ray.direction);
        Cartesian3 barycentricCoords = hitInfo.tri.barycentric(o);

        //Calculate normal vector using barycentric coordinates
        Cartesian3 oNormal= (hitInfo.tri.normals[0].Vector() * barycentricCoords.x) + (hitInfo.tri.normals[1].Vector() * barycentricCoords.y) + (hitInfo.tri.normals[2].Vector() * barycentricCoords.z);

        //If we reach here, then the bounce limit is reached, or the reflectivity value is 0
        //Either way, we calculate the colour of the point using standard methods (Blinn-Phong, shadows etc)
        Homogeneous4 finalColour;
        for (int i = 0; i < renderParameters->lights.size(); i++)
        {
            //Apply modelView matrix to the light position so it's in the right place
            Homogeneous4 lightPosition = scene->getModelView() * renderParameters->lights[i]->GetPositionCenter();
            Homogeneous4 lightColour = renderParameters->lights[i]->GetColor();

            //Determines if a point is in Shadow
            bool inShadow = false;

            //Experimenting with normals
            Homogeneous4 pNormal = hitInfo.tri.normals[0];
            Homogeneous4 qNormal = hitInfo.tri.normals[1];
            Homogeneous4 rNormal = hitInfo.tri.normals[2];

            Cartesian3 normal = ((pNormal * barycentricCoords.x) + (qNormal * barycentricCoords.y) + (rNormal * barycentricCoords.z)).Vector();

            //Adjust the starting position of secondary Ray to prevent the ray intersecting with itself and causing shadow acne
            float epsilon = 0.001;
            Cartesian3 secondaryRayOrigin = o + (epsilon * normal);

            //Initialise the direction of the secondary ray (from the intersection point o to the light position)
            Cartesian3 secondaryRayDirection = (lightPosition.Point() - secondaryRayOrigin).unit();

            //Initialise secondary Ray
            Ray secondaryRay = Ray(secondaryRayOrigin, secondaryRayDirection);
            //Calculate closest intersection to the secondary ray
            Scene::CollisionInfo secondaryHitInfo = scene->closestTriangle(secondaryRay);

            if(secondaryHitInfo.t > 0)
            {
                //Get the distances of the ray origin to the intersected triangle, and the light
                double lengthToTriangle = (secondaryHitInfo.tri.verts->Point() - secondaryRayOrigin).length();
                double lengthToLight = (lightPosition.Point() - secondaryRayOrigin).length();

                //If an object is closer to the ray than the light, then the point o is in shadow
                if((lengthToTriangle < lengthToLight) && !(secondaryHitInfo.tri.shared_material->isLight()))
                {
                    inShadow = true;
                }

            }

            //Calculate colour using Blinn-Phong Model
            Homogeneous4 phong = hitInfo.tri.calculatePhong(lightPosition, lightColour, barycentricCoords, inShadow);
            finalColour = finalColour + phong;
        }

        //We bounce if: the max number of bounces is not reached, and the surface has any sort of reflection
        if (hitInfo.tri.shared_material->reflectivity > 0)
        {

            Ray reflectedRay;
            //Displace by a small amount to prevent acne
            float epsilon = 0.001;
            reflectedRay.origin = o + (epsilon * oNormal);

            //Calculate reflected ray direction using the formula r = r - 2(n.r)n
            reflectedRay.direction = (ray.direction - (2*(ray.direction.dot(oNormal) * oNormal))).unit();

            //Calculate colour * reflectiveness of surface

            //Calculate current colour given its reflectiveness

            finalColour = ((1 - hitInfo.tri.shared_material->reflectivity) * finalColour);
            Homogeneous4 rayColour;

            if(depth != 0)
            {
                rayColour = hitInfo.tri.shared_material->reflectivity * calculateLightforRay(reflectedRay, depth-1);
            }

        //Return the sum of all light colours added
        return finalColour + rayColour;

    }

        return finalColour;
}
    //Not a valid intersection, so return default color (i.e. black)
    return colour;
}

void RaytraceRenderWidget::forceRepaint()
{
    update();
}

Ray RaytraceRenderWidget::calculateRay(int pixelx, int pixely, bool perspective)
{

    //Convert from long to float (required for the division - long rounds down to 0)
    float width = frameBuffer.width;
    float height = frameBuffer.height;

    //Calculate aspect ratio
    float aspect = width / height;
    //x and y are recieved in DCS, so we convert to NDCS

    float xNDS = ((pixelx / width) - 0.5) * 2;
    float yNDS = ((pixely / height) - 0.5) * 2;

    float x, y, z;

    x = xNDS;
    y = yNDS;
    z = -1;

    //We potentially have to factor in the aspect ratio
    if (aspect > 1)
        x = xNDS * aspect;
    else if (aspect < 1)
        y = yNDS / aspect;

    //Perspective - Camera is at the origin
    //Otherwise -> Orthographic
   Ray ray;

    if(perspective)
    {
        ray.origin = {0,0,0};
        ray.direction = {x,y,z};
    }

    else
    {
        ray.origin = {x,y,0};
        ray.direction = {0, 0, z};
    }

    ray.direction = ray.direction.unit();

    return ray;
}

