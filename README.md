# Ray Tracing
## Introduction
 This application involves setting a scene in a rasterisation window, and then raytracing that same scene.

## Compilation

This application was made using Linux and QT 5.15.3, which can be downloaded at:  
https://www.qt.io/download-dev  

If using a Windows machine, the Linux terminal can be accessed by using WSL.
Learn more here: https://learn.microsoft.com/en-us/windows/wsl/

To compile the program, enter the following commands in a terminal:  
    
    qmake -project QT+=opengl LIBS+=-lGLU
    qmake
    make

## Usage
Run the program using `./Ray-Tracing objectFilename materialFilename`.

### Arguments
`objectFilename`
- The object file to be used for raytracing
- .obj file

`materialFilename`
- The material file that accompanies the object 
- .mtl file

### Interface
A basic render of the model can be seen in the left window. The interface contains settings to change how the object is viewed including:
- An arcball to rotate the model
- Sliders that move the mesh in the X and Y directions
- Zoom slider

Press the `Raytrace` button to begin raytracing the scene.
The raytracing can be seen in real time, being drawn on the right window.


The interface allows different settings to be enabled when ray tracing by selecting the relevant checkbox. These settings include:
`Interpolation` - Enable barycentric interpolation
`Phong` - Enable Blinn-Phong shading
`Shadow` - Enable Shadows
`Reflection` - Add reflectivity (can be changed within material file)
`Orthographic` - Render with an orthographic perspective



![Image](assets/ray%20tracing.jpg)