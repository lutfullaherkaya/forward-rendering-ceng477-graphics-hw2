#include "Color.h"
#include <iostream>
#include <iomanip>

using namespace std;

Color::Color() {}

Color::Color(double r, double g, double b) {
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color(const Color &other) {
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

ostream &operator<<(ostream &os, const Color &c) {
    os << fixed << setprecision(0) << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
    return os;
}

Color Color::interpolate(const Color &c, double alpha) const {
    double oneMinusAlpha = 1 - alpha;
    return Color(oneMinusAlpha * r + alpha * c.r,
                 oneMinusAlpha * g + alpha * c.g,
                 oneMinusAlpha * b + alpha * c.b
    );
}

Color Color::interpolate(const Color &c1, const Color &c2, double alpha, double beta, double ceta) const {
    return Color(alpha * r + beta * c1.r + ceta * c2.r,
                 alpha * g + beta * c1.g + ceta * c2.g,
                 alpha * b + beta * c1.b + ceta * c2.b
    );
}
