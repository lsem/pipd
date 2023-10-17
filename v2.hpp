#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>

#include "types.hpp"
#include <QPointF>

struct v2 {
    double x, y;

    v2() {} // uninitialized
    v2(double x, double y) : x(x), y(y) {}
    v2(v2 a, v2 b) : v2(b[0] - a[0], b[1] - a[1]) {}
    v2(QPointF p) : v2(p.x(), p.y()) {}
    v2(Point p) : v2(p.x, p.y) {}
    //    v2(const glm::vec2 &glmv) : v2(glmv.x, glmv.y) {}

    operator QPointF() const { return QPointF(this->x, this->y); }
    operator Point() const { return Point(this->x, this->y); }

    double operator[](std::size_t idx) const {
        assert(idx < 3);
        if (idx == 0) {
            return x;
        } else if (idx == 1) {
            return y;
        } else {
            return {};
        }
    }
};

inline v2 operator*(v2 v, double s) { return v2{v[0] * s, v[1] * s}; }
inline v2 &operator*=(v2 &v, double s) {
    v = v * s;
    return v;
}
inline v2 operator*(double s, v2 v) { return v * s; }
inline v2 operator/(v2 v, double s) { return v2{v[0] / s, v[1] / s}; }
inline v2 operator+(v2 a, v2 b) { return v2{a[0] + b[0], a[1] + b[1]}; }
inline v2 operator-(v2 a, v2 b) { return v2{a[0] - b[0], a[1] - b[1]}; }
inline double dot(v2 a, v2 b) { return a[0] * b[0] + a[1] * b[1]; }
inline double len2(v2 v) { return dot(v, v); }
inline double len(v2 v) { return std::sqrt(len2(v)); }
inline v2 normalized(v2 v) { return v / len(v); }
inline v2 operator-(v2 v) { return v * -1; }

// this is so called scalar cross product which is solving determinant 2x2
// of matrix [v0, v1
//            u0. u1 ]
inline double cross2d(v2 v, v2 u) { return v[0] * u[1] - v[1] * u[0]; }
