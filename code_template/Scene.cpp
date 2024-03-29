#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>
#include <vector>

#include "Scene.h"
#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "tinyxml2.h"
#include "Helpers.h"

using namespace tinyxml2;
using namespace std;


void ForwardRenderingPipeline::doModelingTransformations() {
    for (auto mesh: scene.meshes) {
        int numberOfTransformations = mesh->numberOfTransformations;

        Matrix4 transformationMatrix = getIdentityMatrix();
        for (int i = 0; i < numberOfTransformations; i++) {
            auto transformationId = mesh->transformationIds[i] - 1;
            auto transformationType = mesh->transformationTypes[i];

            if (transformationType == 't') {
                auto tx = scene.translations[transformationId]->tx;
                auto ty = scene.translations[transformationId]->ty;
                auto tz = scene.translations[transformationId]->tz;

                double translation_matrix[4][4] = {
                        {1, 0, 0, tx},
                        {0, 1, 0, ty},
                        {0, 0, 1, tz},
                        {0, 0, 0, 1}
                };
                transformationMatrix = multiplyMatrixWithMatrix(translation_matrix, transformationMatrix);

            } else if (transformationType == 's') {
                auto sx = scene.scalings[transformationId]->sx;
                auto sy = scene.scalings[transformationId]->sy;
                auto sz = scene.scalings[transformationId]->sz;

                double scaling_matrix[4][4] = {
                        {sx, 0,  0,  0},
                        {0,  sy, 0,  0},
                        {0,  0,  sz, 0},
                        {0,  0,  0,  1}
                };
                transformationMatrix = multiplyMatrixWithMatrix(scaling_matrix, transformationMatrix);

            } else if (transformationType == 'r') {
                auto rAngle = scene.rotations[transformationId]->angle;
                auto rx = scene.rotations[transformationId]->ux;
                auto ry = scene.rotations[transformationId]->uy;
                auto rz = scene.rotations[transformationId]->uz;
                // initialize u, declare v and w vectors
                Vec3 u(rx, ry, rz, -1), v, w;
                //finding v vector
                auto minComponent = min(abs(rx), min(abs(ry), abs(rz)));
                if (minComponent == abs(rx)) {
                    v = Vec3(0, -rz, ry, -1);
                } else if (minComponent == abs(ry)) {
                    v = Vec3(-rz, 0, rx, -1);
                } else {
                    v = Vec3(-ry, rx, 0, -1);
                }
                //finding w vector
                w = crossProductVec3(u, v);
                //normalize v,w
                v = normalizeVec3(v);
                w = normalizeVec3(w);
                //finding rotation matrix
                double mInv[4][4] = {
                        {u.x, v.x, w.x, 0},
                        {u.y, v.y, w.y, 0},
                        {u.z, v.z, w.z, 0},
                        {0,   0,   0,   1}
                };

                double m[4][4] = {
                        {u.x, u.y, u.z, 0},
                        {v.x, v.y, v.z, 0},
                        {w.x, w.y, w.z, 0},
                        {0,   0,   0,   1}
                };

                double xRotationMatrix[4][4] = {
                        {1, 0,                        0,                         0},
                        {0, cos(rAngle * M_PI / 180), -sin(rAngle * M_PI / 180), 0},
                        {0, sin(rAngle * M_PI / 180), cos(rAngle * M_PI / 180),  0},
                        {0, 0,                        0,                         1}
                };
                Matrix4 xRotation_m = multiplyMatrixWithMatrix(xRotationMatrix, m);
                Matrix4 mInv_xRotation_m = multiplyMatrixWithMatrix(mInv, xRotation_m);
                transformationMatrix = multiplyMatrixWithMatrix(mInv_xRotation_m, transformationMatrix);
            } else {
                cout << "Error: Unknown transformation type" << endl;
            }

        }

        for (auto &triangle: mesh->triangles) {
            std::vector<Vec3 *> vertices = {&triangle.vertex1, &triangle.vertex2, &triangle.vertex3};
            for (auto &vertexPtr: vertices) {
                Vec3 &vertex = *vertexPtr;
                Vec4 vertices4(vertex.x, vertex.y, vertex.z, 1, -1);
                Vec4 vertexMatrix_m = multiplyMatrixWithVec4(transformationMatrix, vertices4);
                vertex.x = vertexMatrix_m.x;
                vertex.y = vertexMatrix_m.y;
                vertex.z = vertexMatrix_m.z;
            }


        }
    }
}

void ForwardRenderingPipeline::doViewingTransformations() {
    double camToOriginTTemp[4][4] = {
            {1, 0, 0, -camera.pos.x},
            {0, 1, 0, -camera.pos.y},
            {0, 0, 1, -camera.pos.z},
            {0, 0, 0, 1}
    };
    Matrix4 camToOriginT(camToOriginTTemp);

    double camUvwRotateToAlignWithXyzTTemp[4][4] = {
            {camera.u.x, camera.u.y, camera.u.z, 0},
            {camera.v.x, camera.v.y, camera.v.z, 0},
            {camera.w.x, camera.w.y, camera.w.z, 0},
            {0,          0,          0,          1}
    };
    Matrix4 camUvwRotateToAlignWithXyzT(camUvwRotateToAlignWithXyzTTemp);

    /*When points are multiplied with this matrix, their resulting
    coordinates will be with respect to the uvw-e coordinate
    system (i.e. the camera coordinate system)*/

    double l = camera.left;
    double b = camera.bottom;
    double n = camera.near;
    double r = camera.right;
    double t = camera.top;
    double f = camera.far;

    double orthMatrixTemp[4][4] = {
            {2 / (r - l), 0,           0,            -(r + l) / (r - l)},
            {0,           2 / (t - b), 0,            -(t + b) / (t - b)},
            {0,           0,           -2 / (f - n), -(f + n) / (f - n)},
            {0,           0,           0,            1}
    };
    Matrix4 orthMatrix(orthMatrixTemp);

    double p2oMatrixTemp[4][4] = {
            {n, 0, 0,     0},
            {0, n, 0,     0},
            {0, 0, f + n, f * n},
            {0, 0, -1,    0}
    };
    Matrix4 p2oMatrix(p2oMatrixTemp);

    Matrix4 perMatrix = multiplyMatrixWithMatrix(orthMatrix, p2oMatrix);

    double nx = camera.horRes;
    double ny = camera.verRes;
    double vpMatrixTemp[3][4] = {
            {nx / 2.0, 0,      0,   (nx - 1) / 2},
            {0,        ny / 2, 0,   (ny - 1) / 2},
            {0,        0,      0.5, 0.5}
    };
    Matrix4 vpMatrix(vpMatrixTemp);

    /**
     * denemek için
     * the first vertex of first triangle of empty_box.xml
     * -1.0 -1.0 1.0
     * after rotation in y axis by 45 degrees https://keisan.casio.com/exec/system/15362817755710
     * 0 -1 1.414213562
     * after translation id 2
     * 3 -4 -4.585786438
     * after scaling by id 1
     * 15.6 -20.8 -23.8460894776
     */
    for (auto &mesh: scene.meshes) {
        for (auto &triangle: mesh->triangles) {
            Vec4 vertex1(triangle.vertex1.x, triangle.vertex1.y, triangle.vertex1.z, 1, triangle.vertex1.colorId);
            Vec4 vertex2(triangle.vertex2.x, triangle.vertex2.y, triangle.vertex2.z, 1, triangle.vertex2.colorId);
            Vec4 vertex3(triangle.vertex3.x, triangle.vertex3.y, triangle.vertex3.z, 1, triangle.vertex3.colorId);

            vertex1 = multiplyMatrixWithVec4(camToOriginT, vertex1);
            vertex1 = multiplyMatrixWithVec4(camUvwRotateToAlignWithXyzT, vertex1);
            vertex2 = multiplyMatrixWithVec4(camToOriginT, vertex2);
            vertex2 = multiplyMatrixWithVec4(camUvwRotateToAlignWithXyzT, vertex2);
            vertex3 = multiplyMatrixWithVec4(camToOriginT, vertex3);
            vertex3 = multiplyMatrixWithVec4(camUvwRotateToAlignWithXyzT, vertex3);


            if (camera.projectionType == PROJ_ORTHO) {
                vertex1 = multiplyMatrixWithVec4(orthMatrix, vertex1);
                vertex2 = multiplyMatrixWithVec4(orthMatrix, vertex2);
                vertex3 = multiplyMatrixWithVec4(orthMatrix, vertex3);
            } else if (camera.projectionType == PROJ_PERSPECTIVE) {
                vertex1 = multiplyMatrixWithVec4(perMatrix, vertex1);
                vertex2 = multiplyMatrixWithVec4(perMatrix, vertex2);
                vertex3 = multiplyMatrixWithVec4(perMatrix, vertex3);
            }


            if (mesh->type != WIREFRAME) {
                vertex1.perspectiveDivide();
                vertex2.perspectiveDivide();
                vertex3.perspectiveDivide();
                vertex1 = multiplyMatrixWithVec4(vpMatrix, vertex1);
                vertex2 = multiplyMatrixWithVec4(vpMatrix, vertex2);
                vertex3 = multiplyMatrixWithVec4(vpMatrix, vertex3);
                triangle.vertex1 = Vec3(vertex1.x, vertex1.y, vertex1.z, vertex1.colorId);
                triangle.vertex2 = Vec3(vertex2.x, vertex2.y, vertex2.z, vertex2.colorId);
                triangle.vertex3 = Vec3(vertex3.x, vertex3.y, vertex3.z, vertex3.colorId);
                if (camera.projectionType == PROJ_ORTHO) {
                    if (!(scene.cullingEnabled && isCullingExists(triangle))) {//Backface Culling
                        continue;
                    }
                } else if (camera.projectionType == PROJ_PERSPECTIVE) {
                    if (scene.cullingEnabled && isCullingExists(triangle)) {//Backface Culling
                        continue;
                    }
                }

                painter.drawTriangle(triangle);
            }

            if (mesh->type == WIREFRAME) {
                std::pair<Vec4, Vec4> line12(vertex1, vertex2);
                std::pair<Vec4, Vec4> line23(vertex2, vertex3);
                std::pair<Vec4, Vec4> line31(vertex3, vertex1);

                vertex1.perspectiveDivide();
                vertex2.perspectiveDivide();
                vertex3.perspectiveDivide();
                vertex1 = multiplyMatrixWithVec4(vpMatrix, vertex1);
                vertex2 = multiplyMatrixWithVec4(vpMatrix, vertex2);
                vertex3 = multiplyMatrixWithVec4(vpMatrix, vertex3);
                triangle.vertex1 = Vec3(vertex1.x, vertex1.y, vertex1.z, vertex1.colorId);
                triangle.vertex2 = Vec3(vertex2.x, vertex2.y, vertex2.z, vertex2.colorId);
                triangle.vertex3 = Vec3(vertex3.x, vertex3.y, vertex3.z, vertex3.colorId);
                if (scene.cullingEnabled && isCullingExists(triangle)) {//Backface Culling
                    continue;
                }

                bool line1Visible = clipping(line12.first, line12.second);
                bool line2Visible = clipping(line23.first, line23.second);
                bool line3Visible = clipping(line31.first, line31.second);



                line12.first.perspectiveDivide();
                line12.second.perspectiveDivide();
                line23.first.perspectiveDivide();
                line23.second.perspectiveDivide();
                line31.first.perspectiveDivide();
                line31.second.perspectiveDivide();
                // L01
                line12.first = multiplyMatrixWithVec4(vpMatrix, line12.first);
                line12.second = multiplyMatrixWithVec4(vpMatrix, line12.second);

                // L12
                line23.first = multiplyMatrixWithVec4(vpMatrix, line23.first);
                line23.second = multiplyMatrixWithVec4(vpMatrix, line23.second);

                // L20
                line31.first = multiplyMatrixWithVec4(vpMatrix, line31.first);
                line31.second = multiplyMatrixWithVec4(vpMatrix, line31.second);


                if (line1Visible) painter.drawLine(line12.first, line12.second);
                if (line2Visible) painter.drawLine(line23.first, line23.second);
                if (line3Visible) painter.drawLine(line31.first, line31.second);
            }
        }
    }

}

void ForwardRenderingPipeline::doRasterization() {
/*    for (auto &mesh: scene.meshes) {
        for (auto &triangle: mesh->triangles) {

            if (scene.cullingEnabled && isCullingExists(triangle)) {//Backface Culling
                continue;
            }

            if (mesh->type == WIREFRAME) {
                bool line1Visible = clipping(triangle.vertex1, triangle.vertex2);
                bool line2Visible = clipping(triangle.vertex2, triangle.vertex3);
                bool line3Visible = clipping(triangle.vertex3, triangle.vertex1);
                painter.drawLine(triangle.vertex1, triangle.vertex2);
                painter.drawLine(triangle.vertex2, triangle.vertex3);
                painter.drawLine(triangle.vertex3, triangle.vertex1);
            } else {
                //painter.drawTriangle(triangle);
            }

        }
    }*/
}

ForwardRenderingPipeline::ForwardRenderingPipeline(Scene &scene1, Camera &camera1) : scene(scene1),
                                                                                     camera(camera1),
                                                                                     painter(scene, camera) {


}

bool ForwardRenderingPipeline::isCullingExists(Triangle &triangle) {
    Vec3 vertex1 = triangle.vertex1;
    Vec3 vertex2 = triangle.vertex2;
    Vec3 vertex3 = triangle.vertex3;

    Vec3 v1 = subtractVec3(vertex2, vertex1);
    Vec3 v2 = subtractVec3(vertex3, vertex1);
    Vec3 normal = normalizeVec3(crossProductVec3(v1, v2));
    bool culling_exists = dotProductVec3(normal, vertex1) < 0;
    return culling_exists;
}

bool ForwardRenderingPipeline::isVisible(double den, double num, double &t_E, double &t_L) {
    double t = num / den;
    if (den > 0) {
        if (t > t_L) {
            return false;
        }
        if (t > t_E) {
            t_E = t;
        }
    } else if (den < 0) {
        if (t < t_E) {
            return false;
        }
        if (t < t_L) {
            t_L = t;
        }
    } else if (num > 0) {
        return false;
    }
    return true;
}


bool ForwardRenderingPipeline::clipping(Vec4 &vertex1, Vec4 &vertex2) { //Liang-Barsky Algorithm is implemented
    Color *color1 = scene.colorsOfVertices[vertex1.colorId - 1];
    Color *color2 = scene.colorsOfVertices[vertex2.colorId - 1];

    double t_E = 0;
    double t_L = 1;

    double dx = vertex2.x - vertex1.x;
    double dy = vertex2.y - vertex1.y;
    double dz = vertex2.z - vertex1.z;

    Color dc;
    dc.r = color2->r - color1->r;
    dc.g = color2->g - color1->g;
    dc.b = color2->b - color1->b;

    double x_min = -1;
    double x_max = 1;
    double y_min = -1;
    double y_max = 1;
    double z_min = -1;
    double z_max = 1;

    bool lineVisible = true;
    if (isVisible(dx, x_min - vertex1.x, t_E, t_L)) {//left
        if (isVisible(-dx, vertex1.x - x_max, t_E, t_L)) {//right
            if (isVisible(dy, y_min - vertex1.y, t_E, t_L)) {//bottom
                if (isVisible(-dy, vertex1.y - y_max, t_E, t_L)) {//top
                    if (isVisible(dz, z_min - vertex1.z, t_E, t_L)) {//front
                        if (isVisible(-dz, vertex1.z - z_max, t_E, t_L)) {//back
                            lineVisible = true;
                            if (t_L < 1) {
                                vertex2.x = vertex1.x + t_L * dx;
                                vertex2.y = vertex1.y + t_L * dy;
                                vertex2.z = vertex1.z + t_L * dz;
                                color2->r = color1->r + t_L * dc.r;
                                color2->g = color1->g + t_L * dc.g;
                                color2->b = color1->b + t_L * dc.b;
                            }
                            if (t_E > 0) {
                                vertex1.x = vertex1.x + t_E * dx;
                                vertex1.y = vertex1.y + t_E * dy;
                                vertex1.z = vertex1.z + t_E * dz;
                                color1->r = color1->r + t_E * dc.r;
                                color1->g = color1->g + t_E * dc.g;
                                color1->b = color1->b + t_E * dc.b;
                            }
                        }
                    }
                }
            }
        }
    }

    return lineVisible;
}

void Painter::draw(int x, int y, Color color) {
    if (onCanvas(x, y)) {
        scene.image[x][y] = color;
    }

}

void Painter::drawLine(Vec3 &src, Vec3 &dest) {
    // midpoint algorithm
    int x1 = src.x;
    int x2 = dest.x;
    int y1 = src.y;
    int y2 = dest.y;

    int dy = y2 - y1;
    int dx = x2 - x1;
    double m = (double) dy / dx;
    if (dx < 0) {
        drawLine(dest, src);
    } else {

        int slopeSign;

        if (dy < 0) {
            slopeSign = -1;
            dy = -dy;
        } else {
            slopeSign = 1;
        }

        const Color &c0 = *scene.colorsOfVertices[src.colorId - 1];
        const Color &c1 = *scene.colorsOfVertices[dest.colorId - 1];
        if (-1 < m && m < 1) {

            int y = y1;
            int d = 2 * -dy + dx;
            for (int x = x1; x < x2; ++x) {
                double alpha = (double) (x - x1) / (x2 - x1);
                draw(x, y, c0.interpolate(c1, alpha));
                d += 2 * -dy;
                if (d < 0) { // choose NE
                    y += slopeSign;
                    d += 2 * dx;
                } else {}// choose
            }
        } else {
            int x = x1;
            int d = 2 * -dx + dy;
            for (int y = y1; y != y2; y += slopeSign) {
                double alpha = (double) (y - y1) / (y2 - y1);
                draw(x, y, c0.interpolate(c1, alpha));
                d += 2 * -dx;
                if (d < 0) { // choose NE
                    x++;
                    d += 2 * dy;
                } else {}// choose E
            }
        }
    }


}

bool Painter::onCanvas(int x, int y) const {
    return 0 < x && x < camera.horRes &&
           0 < y && y < camera.verRes;
}


Painter::Painter(Scene &scene, Camera &camera) : scene(scene), camera(camera) {}

void Painter::drawTriangle(Triangle &triangle) {
    int left = std::min(triangle.vertex1.x, std::min(triangle.vertex2.x, triangle.vertex3.x));
    int right = std::max(triangle.vertex1.x, std::max(triangle.vertex2.x, triangle.vertex3.x));
    int top = std::min(triangle.vertex1.y, std::min(triangle.vertex2.y, triangle.vertex3.y));
    int bottom = std::max(triangle.vertex1.y, std::max(triangle.vertex2.y, triangle.vertex3.y));
    const Color &c0 = *scene.colorsOfVertices[triangle.vertex1.colorId - 1];
    const Color &c1 = *scene.colorsOfVertices[triangle.vertex2.colorId - 1];
    const Color &c2 = *scene.colorsOfVertices[triangle.vertex3.colorId - 1];
    for (int x = left; x <= right; ++x) {
        for (int y = top; y <= bottom; ++y) {
            double alpha = triangle.f12(x, y) / (double) triangle.f12(triangle.vertex1.x, triangle.vertex1.y);
            double beta = triangle.f20(x, y) / (double) triangle.f20(triangle.vertex2.x, triangle.vertex2.y);
            double ceta = triangle.f01(x, y) / (double) triangle.f01(triangle.vertex3.x, triangle.vertex3.y);
            if (alpha >= 0 && beta >= 0 && ceta >= 0) {
                draw(x, y, c0.interpolate(c1, c2, alpha, beta, ceta));
            }
        }
    }
}

void Painter::drawLine(Vec4 &src, Vec4 &dest) {
    Vec3 src3(src.x, src.y, src.z, src.colorId);
    Vec3 dest3(dest.x, dest.y, dest.z, dest.colorId);
    drawLine(src3, dest3);
}


/*
	Transformations, clipping, culling, rasterization are done here.
	You may define helper functions.
*/
void Scene::forwardRenderingPipeline(Camera *camera) {
    auto pipe = ForwardRenderingPipeline(*this, *camera);
    pipe.doModelingTransformations();
    pipe.doViewingTransformations();
    pipe.doRasterization();

}

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath) {
    const char *str;
    XMLDocument xmlDoc;
    XMLElement *pElement;

    xmlDoc.LoadFile(xmlPath);

    XMLNode *pRoot = xmlDoc.FirstChild();

    // read background color
    pElement = pRoot->FirstChildElement("BackgroundColor");
    str = pElement->GetText();
    sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

    // read culling
    pElement = pRoot->FirstChildElement("Culling");
    if (pElement != NULL) {
        str = pElement->GetText();

        if (strcmp(str, "enabled") == 0) {
            cullingEnabled = true;
        } else {
            cullingEnabled = false;
        }
    }

    // read cameras
    pElement = pRoot->FirstChildElement("Cameras");
    XMLElement *pCamera = pElement->FirstChildElement("Camera");
    XMLElement *camElement;
    while (pCamera != NULL) {
        Camera *cam = new Camera();

        pCamera->QueryIntAttribute("id", &cam->cameraId);

        // read projection type
        str = pCamera->Attribute("type");

        if (strcmp(str, "orthographic") == 0) {
            cam->projectionType = 0;
        } else {
            cam->projectionType = 1;
        }

        camElement = pCamera->FirstChildElement("Position");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf", &cam->pos.x, &cam->pos.y, &cam->pos.z);

        camElement = pCamera->FirstChildElement("Gaze");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf", &cam->gaze.x, &cam->gaze.y, &cam->gaze.z);

        camElement = pCamera->FirstChildElement("Up");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf", &cam->v.x, &cam->v.y, &cam->v.z);

        cam->gaze = normalizeVec3(cam->gaze);
        cam->u = crossProductVec3(cam->gaze, cam->v);
        cam->u = normalizeVec3(cam->u);

        cam->w = inverseVec3(cam->gaze);
        cam->v = crossProductVec3(cam->u, cam->gaze);
        cam->v = normalizeVec3(cam->v);

        camElement = pCamera->FirstChildElement("ImagePlane");
        str = camElement->GetText();
        sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
               &cam->left, &cam->right, &cam->bottom, &cam->top,
               &cam->near, &cam->far, &cam->horRes, &cam->verRes);

        camElement = pCamera->FirstChildElement("OutputName");
        str = camElement->GetText();
        cam->outputFileName = string(str);

        cameras.push_back(cam);

        pCamera = pCamera->NextSiblingElement("Camera");
    }

    // read vertices
    pElement = pRoot->FirstChildElement("Vertices");
    XMLElement *pVertex = pElement->FirstChildElement("Vertex");
    int vertexId = 1;

    while (pVertex != NULL) {
        Vec3 *vertex = new Vec3();
        Color *color = new Color();

        vertex->colorId = vertexId;

        str = pVertex->Attribute("position");
        sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

        str = pVertex->Attribute("color");
        sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

        vertices.push_back(vertex);
        colorsOfVertices.push_back(color);

        pVertex = pVertex->NextSiblingElement("Vertex");

        vertexId++;
    }

    // read translations
    pElement = pRoot->FirstChildElement("Translations");
    XMLElement *pTranslation = pElement->FirstChildElement("Translation");
    while (pTranslation != NULL) {
        Translation *translation = new Translation();

        pTranslation->QueryIntAttribute("id", &translation->translationId);

        str = pTranslation->Attribute("value");
        sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

        translations.push_back(translation);

        pTranslation = pTranslation->NextSiblingElement("Translation");
    }

    // read scalings
    pElement = pRoot->FirstChildElement("Scalings");
    XMLElement *pScaling = pElement->FirstChildElement("Scaling");
    while (pScaling != NULL) {
        Scaling *scaling = new Scaling();

        pScaling->QueryIntAttribute("id", &scaling->scalingId);
        str = pScaling->Attribute("value");
        sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

        scalings.push_back(scaling);

        pScaling = pScaling->NextSiblingElement("Scaling");
    }

    // read rotations
    pElement = pRoot->FirstChildElement("Rotations");
    XMLElement *pRotation = pElement->FirstChildElement("Rotation");
    while (pRotation != NULL) {
        Rotation *rotation = new Rotation();

        pRotation->QueryIntAttribute("id", &rotation->rotationId);
        str = pRotation->Attribute("value");
        sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

        rotations.push_back(rotation);

        pRotation = pRotation->NextSiblingElement("Rotation");
    }

    // read meshes
    pElement = pRoot->FirstChildElement("Meshes");

    XMLElement *pMesh = pElement->FirstChildElement("Mesh");
    XMLElement *meshElement;
    while (pMesh != NULL) {
        Mesh *mesh = new Mesh();

        pMesh->QueryIntAttribute("id", &mesh->meshId);

        // read projection type
        str = pMesh->Attribute("type");

        if (strcmp(str, "wireframe") == 0) {
            mesh->type = 0;
        } else {
            mesh->type = 1;
        }

        // read mesh transformations
        XMLElement *pTransformations = pMesh->FirstChildElement("Transformations");
        XMLElement *pTransformation = pTransformations->FirstChildElement("Transformation");

        while (pTransformation != NULL) {
            char transformationType;
            int transformationId;

            str = pTransformation->GetText();
            sscanf(str, "%c %d", &transformationType, &transformationId);

            mesh->transformationTypes.push_back(transformationType);
            mesh->transformationIds.push_back(transformationId);

            pTransformation = pTransformation->NextSiblingElement("Transformation");
        }

        mesh->numberOfTransformations = mesh->transformationIds.size();

        // read mesh faces
        char *row;
        char *clone_str;
        int v1, v2, v3;
        XMLElement *pFaces = pMesh->FirstChildElement("Faces");
        str = pFaces->GetText();
        clone_str = strdup(str);

        row = strtok(clone_str, "\n");
        while (row != NULL) {
            int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

            if (result != EOF) {
                mesh->triangles.push_back(Triangle(v1, v2, v3));
            }
            row = strtok(NULL, "\n");
        }
        mesh->numberOfTriangles = mesh->triangles.size();
        meshes.push_back(mesh);

        pMesh = pMesh->NextSiblingElement("Mesh");
    }
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera) {
    if (this->image.empty()) {
        for (int i = 0; i < camera->horRes; i++) {
            vector<Color> rowOfColors;

            for (int j = 0; j < camera->verRes; j++) {
                rowOfColors.push_back(this->backgroundColor);
            }

            this->image.push_back(rowOfColors);
        }
    } else {
        for (int i = 0; i < camera->horRes; i++) {
            for (int j = 0; j < camera->verRes; j++) {
                this->image[i][j].r = this->backgroundColor.r;
                this->image[i][j].g = this->backgroundColor.g;
                this->image[i][j].b = this->backgroundColor.b;
            }
        }
    }
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value) {
    if (value >= 255.0)
        return 255;
    if (value <= 0.0)
        return 0;
    return (int) (value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera) {
    ofstream fout;

    fout.open(camera->outputFileName.c_str());

    fout << "P3" << endl;
    fout << "# " << camera->outputFileName << endl;
    fout << camera->horRes << " " << camera->verRes << endl;
    fout << "255" << endl;

    for (int j = camera->verRes - 1; j >= 0; j--) {
        for (int i = 0; i < camera->horRes; i++) {
            fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
                 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
                 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
        }
        fout << endl;
    }
    fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType) {
    string command;

    // call command on Ubuntu
    if (osType == 1) {
        command = "convert " + ppmFileName + " " + ppmFileName + ".png";
        system(command.c_str());
    }

        // call command on Windows
    else if (osType == 2) {
        command = "magick convert " + ppmFileName + " " + ppmFileName + ".png";
        system(command.c_str());
    }

        // default action - don't do conversion
    else {
    }
}