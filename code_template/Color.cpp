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
