#ifndef __VEC3_H__
#define __VEC3_H__

#include <iostream>
using namespace std;

class Vec3
{
public:
    double x, y, z;
    int colorId;

    Vec3();
    Vec3(double x, double y, double z, int colorId);
    Vec3(const Vec3 &other);

    double getElementAt(int index);
    
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v);

    double &operator[](int i) {
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default:
                return x;
        }
    }


};

#endif