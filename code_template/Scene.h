#ifndef _SCENE_H_
#define _SCENE_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "Vec4.h"

using namespace std;

class Scene {
public:
    Color backgroundColor;
    bool cullingEnabled;

    vector<vector<Color>> image;
    vector<Camera *> cameras;
    vector<Vec3 *> vertices;
    vector<Color *> colorsOfVertices;
    vector<Scaling *> scalings;
    vector<Rotation *> rotations;
    vector<Translation *> translations;
    vector<Mesh *> meshes;

    Scene(const char *xmlPath);

    void initializeImage(Camera *camera);

    void forwardRenderingPipeline(Camera *camera);

    int makeBetweenZeroAnd255(double value);

    void writeImageToPPMFile(Camera *camera);

    void convertPPMToPNG(string ppmFileName, int osType);
};

class Painter {
public:
    Scene &scene;
    Camera &camera;

    Painter(Scene &scene, Camera &camera);

    void draw(int x, int y, Color color);

    void drawLine(Vec3 &src, Vec3 &dest);
    void drawLine(Vec4 &src, Vec4 &dest);

    void drawTriangle(Triangle &triangle);

    bool onCanvas(int x, int y) const;
};

class ForwardRenderingPipeline {
public:
    Scene &scene;
    Camera &camera;
    Painter painter;

    ForwardRenderingPipeline(Scene &scene, Camera &camera);

    bool isVisible(double den, double num, double& t_E, double& t_L);

    bool clipping(Vec4& vertex1, Vec4& vertex2);

    bool isCullingExists(Triangle &triangle);

    void doModelingTransformations();

    void doViewingTransformations();

    void doRasterization();

};


#endif
