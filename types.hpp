#pragma once
#include <QDebug>
#include <string>

enum class Tool { hand, select, draw_point, draw_line };

QDebug &operator<<(QDebug &os, Tool t);

struct Point {
    double x = 0.0;
    double y = 0.0;

    Point(double x, double y) : x(x), y(y) {}
    Point() = default;
};

QDebug &operator<<(QDebug &os, Point t);

// PointObj is a "Point Object". Anything that may be important for our drawing. It may be a corner
// of a house or room, or other important vertex. It is different from just geometry that it can
// have an ID, description, etc.. Also, it has different handling and representation in the canvas.
// E.g. we can link vertex of a line to point and by moving point we also move all accociated
// vertices.
struct PointObj : public Point {
    PointObj(double x, double y, std::string id) : Point(x, y), id(std::move(id)) {}
    PointObj(Point p, std::string id) : Point(p), id(std::move(id)) {}
    std::string id;
};

struct Line {
    Point a;
    Point b;
};

struct LineObj {
    Line l;
    std::string id;
};

// Defines by upper-left corner and width, height.
struct Rect {
    double x;
    double y;
    double width;
    double height;
};
