#pragma once
#include <QDebug>
#include <QPointF>
#include <optional>
#include <string>

namespace ObjFlags {
enum {
    selected = 0x01,
    moving = 0x02,
    howered = 0x04,
    a_endpoint_move = 0x08,
    b_endpoint_move = 0x10,
    a_endpoint_move_howered = 0x20,
    b_endpoint_move_howered = 0x40
};
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
    Line(Point a, Point b) : a(a), b(b) {}
    Line() = default;

    std::tuple<Point, Point> endpoints() const { return {a, b}; }
};

struct LineObj {
    Line l;
    Line shadow_l;
    std::string id;
    unsigned flags = 0;

    // Line always refers to some endpoints, implicitly or explicitly created.
    std::optional<std::string> endpoint_a_ref;
    std::optional<std::string> endpoint_b_ref;
};

// What if line's own endpoint is rendered differently and handled differently? Meaning, that we can
// move it but it does not have this sticky semantics? But anytime we can turn this endpoint into
// separate Point with own ID.

// What if we just create separate endpoints in the model and somehow connect them in the model?
// BTW, how we can connect them?
// we can have separte list of connections. This way rendering does not need to lookup for the
// connection and just render what we have: points and lines.
//
// Another question, can we put a point on a line and then connect with another line?
// It seems to be useful because we may want to denote a wall and only then draw it. We can also
// want to slide this point along the line. Or, we may want to slide a wall using M tool and slide
// two endpoints along the lines.

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
