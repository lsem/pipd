#pragma once
#include <QDebug>
#include <QPointF>
#include <string>

namespace ObjFlags {
enum { selected = 0x01, moving = 0x02 };
};

enum class Tool { hand, select, draw_point, draw_line, move };

QDebug &operator<<(QDebug &os, Tool t);

struct Point {
    double x = 0.0;
    double y = 0.0;

    Point(double x, double y) : x(x), y(y) {}
    Point() = default;
};

QDebug &operator<<(QDebug &os, Point t);

struct PointObj {
    PointObj(Point p, std::string id) : pt(p), id(std::move(id)) {}
    Point pt;
    std::string id;
};

struct Line {
    Point a;
    Point b;

    std::tuple<Point, Point> endpoints() const { return {a, b}; }
};

struct LineObj {
    Line l;
    Line shadow_l;
    std::string id;
    unsigned flags = 0;
};

// Defines by upper-left corner and width, height.
struct Rect {
    double x;
    double y;

    Point upper_left_corner() const { return {x, y}; }
    Point center() const { return {x + width / 2, y + height / 2}; }
    double width;
    double height;
};

struct Model {
    std::vector<PointObj> points;
    std::vector<LineObj> lines;
};
