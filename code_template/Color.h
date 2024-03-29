#ifndef __COLOR_H__
#define __COLOR_H__

#include <iostream>

class Color
{
public:
    double r, g, b;

    Color();
    Color(double r, double g, double b);
    Color(const Color &other);
    friend std::ostream& operator<<(std::ostream& os, const Color& c);
    Color interpolate(const Color &c, double alpha) const;
    Color interpolate(const Color &c1, const Color &c2, double alpha, double beta, double ceta) const;
};

#endif