#pragma once
#include <QDebug>
#include <QPointF>
#include <optional>
#include <string>
#include <variant>

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

    // duct handling,
    duct_a_endpoint_howered = 0x8000,
    duct_b_endpoint_howered = 0x10000,
    fitting_a_endpoint_howered = 0x20000,
    fitting_b_endpoint_howered = 0x40000,
};
}

enum class Tool { hand, select, draw_point, draw_line, move, guide, rectangle, duct, adapter };

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
    Point upper_right_corner() const { return {x + width, y}; }
    Point center() const { return {x + width / 2, y + height / 2}; }
    Point bottom_right_corner() const { return {x + width, y + height}; }
    Point bottom_left_corner() const { return {x, y + height}; }

    Line top_line() const { return Line(Point(x, y), Point(x + width, y)); }
    Line bottom_line() const { return Line(Point(x, y + height), Point(x + width, y + height)); }
    Line left_line() const { return Line(Point(x, y), Point(x, y + height)); }
    Line right_line() const { return Line(Point(x + width, y), Point(x + width, y + height)); }

    // TODO: Top line move is restricted to one dimension if we want to keep rectangles. But it may
    // be more useful to have free form so that we can create parallelograms. But parallelgram would
    // require showing angle.
    //
    void move_top_line(double dy) {
        y += dy;
        height -= dy;
    }
    void move_bottom_line(double dy) { height += dy; }
    void move_left_line(double dx) {
        x += dx;
        width -= dx;
    }
    void move_right_line(double dx) { width += dx; }

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
    Rect shadow_rect;

    unsigned flags = 0;
};

struct Duct {
    unsigned size_mm;
    // Note, we don't have explicit length.
    Point begin;
    Point end;

    uint32_t flags;
};

// Adapts one size to another side. This is generic component for adapter, actual adapter is going
// to be created from catalogue.
struct Adapter {
    Point begin;
    Point end;
    double begin_d = 0.0;
    double end_d = 0.0;
};

struct Split3 {
    Point begin;
    Point end;
};

struct Fitting {
    std::variant<Adapter, Split3> fitting_variant;
    Point center;
    uint32_t flags;
};

struct Model {
    std::vector<PointObj> points;
    std::vector<LineObj> lines;
    std::vector<GuideObj> guides;
    std::vector<RectObj> rects;

    // Ducts model allow to have any configuration including completely disconnected ducts,fittings
    // and other elements. On practise however, we are not going to allow creation of any model.
    std::vector<Duct> ducts;
    std::vector<Fitting> fittings;

    // What about connections between ducts and fittings?
    // connections?
    // What is the use case of our connections?
    // ducts should be connected to fittings.
};

// Next question, how we are supposed to work with these ducts?
// How to model a joint of two? One possible way for doing this is to have a new thing called
// Fitting. This way we connect fitting to three ducts. Fitting has its own representation and its
// own constraints (like minimum size), it also has a dimater and aerodynamic characteristics.
