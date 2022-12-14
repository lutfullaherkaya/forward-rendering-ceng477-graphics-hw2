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
            std::vector<Vec3 *> vertices = {&triangle.vertex1, &triangle.vertex2, &triangle.vertex3};
            for (auto &vertexPtr: vertices) {
                Vec3 &vertex = *vertexPtr;
                Vec4 vertex4(vertex.x, vertex.y, vertex.z, 1, -1);
                /*vertex4.x = 15.6;
                vertex4.y = -20.8;
                vertex4.z = -23.8460894776;*/
                vertex4 = multiplyMatrixWithVec4(camToOriginT, vertex4);
                vertex4 = multiplyMatrixWithVec4(camUvwRotateToAlignWithXyzT, vertex4);
                if (camera.projectionType == PROJ_ORTHO) {
                    vertex4 = multiplyMatrixWithVec4(orthMatrix, vertex4);
                } else if (camera.projectionType == PROJ_PERSPECTIVE) {
                    vertex4 = multiplyMatrixWithVec4(perMatrix, vertex4);
                }
                vertex4.perspectiveDivide();
                vertex4 = multiplyMatrixWithVec4(vpMatrix, vertex4);
                vertex.x = vertex4.x;
                vertex.y = vertex4.y;
                vertex.z = vertex4.z;
            }
        }
    }

}

void ForwardRenderingPipeline::doRasterization() {
    for (auto &mesh: scene.meshes) {
        for (auto &triangle: mesh->triangles) {
            if (mesh->type == WIREFRAME) {
                painter.drawLine(triangle.vertex1, triangle.vertex2);
                painter.drawLine(triangle.vertex2, triangle.vertex3);
                painter.drawLine(triangle.vertex3, triangle.vertex1);
            } else {
                // todo:
            }

        }
    }
}

ForwardRenderingPipeline::ForwardRenderingPipeline(Scene &scene1, Camera &camera1) : scene(scene1),
                                                                                     camera(camera1),
                                                                                     painter(scene, camera) {


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

    // todo: put x,y to int point
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

        const Color &c0 = *scene.colorsOfVertices[src.colorId-1];
        const Color &c1 = *scene.colorsOfVertices[dest.colorId-1];
        if (-1 < m && m < 1) {

            int y = y1;
            int d = 2 * -dy + dx;
            for (int x = x1; x < x2; ++x) {
                double alpha = (double)(x - x1) / (x2 - x1);
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
                double alpha = (double)(y - y1) / (y2 - y1);
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