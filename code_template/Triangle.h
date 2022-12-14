#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "Vec3.h"
class Triangle
{
public:
    int vertexIds[3];
    Vec3 vertex1;
    Vec3 vertex2;
    Vec3 vertex3;

    Triangle();
    Triangle(int vid1, int vid2, int vid3);
    Triangle(const Triangle &other);

    int getFirstVertexId();
    int getSecondVertexId();
    int getThirdVertexId();

    void setFirstVertexId(int vid);
    void setSecondVertexId(int vid);
    void setThirdVertexId(int vid);

    bool contains(int x, int y);

    int f01(int x, int y);
    int f12(int x, int y);
    int f20(int x, int y);

};


#endif