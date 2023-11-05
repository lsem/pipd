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

    // line handling
    a_endpoint_move = 0x08,
    b_endpoint_move = 0x10,
    a_endpoint_move_howered = 0x20,
    b_endpoint_move_howered = 0x40,

    // rect handling
    top_rect_line_move_howered = 0x80,
    top_rect_line_move = 0x100,
    bottom_rect_line_move_howered = 0x200,
    bottom_rect_line_move = 0x400,
    left_rect_line_move_howered = 0x800,
    left_rect_line_move = 0x1000,
    right_rect_line_move_howered = 0x2000,
    right_rect_line_move = 0x4000,
};
}

enum class Tool { hand, select, draw_point, draw_line, move, guide, rectangle };

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

QDebug &operator<<(QDebug &os, Line l);

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

    // Constructs from any two points.
    static Rect from_two_points(Point p1, Point p2) {
        // we need to find top left from any two points.
        if (p1.x < p2.x) {
            if (p1.x < p2.y) {
                // p1 is topleft.
                return Rect{p1.x, p1.y, p2.x - p1.x, p2.y - p1.y};
            } else {
                // p1 is bottom left.
                return Rect{p1.x, p2.y, p2.x - p1.x, p1.y - p2.y};
            }
        } else {
            // p2.x <= p1.x
            if (p2.y <= p1.y) {
                // p2 is topleft.
                return Rect{p2.x, p2.y, p1.x - p2.x, p1.y - p2.y};
            } else {
                // p2 is bottom left
                return Rect{p2.x, p1.y, p1.x - p2.x, p2.y - p1.y};
            }
        }
    }

    static Rect from_center_and_dimensions(Point center, double width, double height) {
        return Rect{center.x - width / 2.0, center.y - height / 2.0, width, height};
    }

    Point upper_left_corner() const { return {x, y}; }
    Point center() const { return {x + width / 2, y + height / 2}; }

    Line top_line() const { return Line(Point(x, y), Point(x + width, y)); }
    Line bottom_line() const { return Line(Point(x, y + height), Point(x + width, y + height)); }
    Line left_line() const { return Line(Point(x, y), Point(x, y + height)); }
    Line right_line() const { return Line(Point(x + width, y), Point(x + width, y + height)); }

    double x;
    double y;
    double width;
    double height;
};

struct GuideObj {
    Line line; // shouln't this be called geometry?
};

struct RectObj {
    Rect rect; // shouln't this be called geometry?

    unsigned flags = 0;
};

struct Model {
    std::vector<PointObj> points;
    std::vector<LineObj> lines;
    std::vector<GuideObj> guides;
    std::vector<RectObj> rects;
};
