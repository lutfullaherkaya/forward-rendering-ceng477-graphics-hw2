#include <iostream>
#include <string>
#include <vector>
#include "Scene.h"
#include "Matrix4.h"
#include "Helpers.h"

using namespace std;

Scene *scene;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Please run the rasterizer as:" << endl
             << "\t./rasterizer <input_file_name>" << endl;
        return 1;
    } else {
        const char *xmlPath = argv[1];

        scene = new Scene(xmlPath);

        for (int i = 0; i < scene->cameras.size(); i++) {
            // for making sure each vertex is seperate between meshes in transformations
            for (auto &mesh: scene->meshes) {
                for (auto &triangle: mesh->triangles) {
                    triangle.vertex1 = *(scene->vertices[triangle.vertexIds[0] - 1]);
                    triangle.vertex2 = *(scene->vertices[triangle.vertexIds[1] - 1]);
                    triangle.vertex3 = *(scene->vertices[triangle.vertexIds[2] - 1]);
                }
            }

            // initialize image with basic values
            scene->initializeImage(scene->cameras[i]);

            // do forward rendering pipeline operations
            scene->forwardRenderingPipeline(scene->cameras[i]);

            // generate PPM file
            scene->writeImageToPPMFile(scene->cameras[i]);

            // Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
            // Notice that os_type is not given as 1 (Ubuntu) or 2 (Windows), below call doesn't do conversion.
            // Change os_type to 1 or 2, after being sure that you have ImageMagick installed.
            scene->convertPPMToPNG(scene->cameras[i]->outputFileName, 2);
        }

        return 0;
    }
}