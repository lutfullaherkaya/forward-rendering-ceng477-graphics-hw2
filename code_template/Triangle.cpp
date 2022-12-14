#include "Triangle.h"

Triangle::Triangle() {
    this->vertexIds[0] = -1;
    this->vertexIds[1] = -1;
    this->vertexIds[2] = -1;
}

Triangle::Triangle(int vid1, int vid2, int vid3) {
    this->vertexIds[0] = vid1;
    this->vertexIds[1] = vid2;
    this->vertexIds[2] = vid3;
}

Triangle::Triangle(const Triangle &other) {
    this->vertexIds[0] = other.vertexIds[0];
    this->vertexIds[1] = other.vertexIds[1];
    this->vertexIds[2] = other.vertexIds[2];
}

// getters
int Triangle::getFirstVertexId() {
    return this->vertexIds[0];
}

int Triangle::getSecondVertexId() {
    return this->vertexIds[1];
}

int Triangle::getThirdVertexId() {
    return this->vertexIds[2];
}

// setters
void Triangle::setFirstVertexId(int vid) {
    this->vertexIds[0] = vid;
}

void Triangle::setSecondVertexId(int vid) {
    this->vertexIds[1] = vid;
}

void Triangle::setThirdVertexId(int vid) {
    this->vertexIds[2] = vid;
}

bool Triangle::contains(int x, int y) {
    return f01(x, y) <= 0 && f12(x, y) <= 0 && f20(x, y) <= 0;

}

int Triangle::f01(int x, int y) {
    int y0 = vertex1.y;
    int y1 = vertex2.y;
    int x0 = vertex1.x;
    int x1 = vertex2.x;
    return x * (y0 - y1) + y * (x1 - x0) + x0 * y1 - x1 * y0;
}

int Triangle::f12(int x, int y) {
    int y1 = vertex2.y;
    int y2 = vertex3.y;
    int x1 = vertex2.x;
    int x2 = vertex3.x;
    return x * (y1 - y2) + y * (x2 - x1) + x1 * y2 - x2 * y1;
}

int Triangle::f20(int x, int y) {
    int y0 = vertex1.y;
    int y2 = vertex3.y;
    int x0 = vertex1.x;
    int x2 = vertex3.x;
    return x * (y2 - y0) + y * (x0 - x2) + x2 * y0 - x0 * y2;
}