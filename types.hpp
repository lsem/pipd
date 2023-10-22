#pragma once
#include <QDebug>
#include <QPointF>
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

struct PointObj {
    PointObj(Point p, std::string id) : pt(p), id(std::move(id)) {}
    Point pt;
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

    Point upper_left_corner() const { return {x, y}; }
    double width;
    double height;
};

struct Model {
    std::vector<PointObj> points;
    std::vector<LineObj> lines;
};
