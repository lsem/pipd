#include "types.hpp"

QDebug &operator<<(QDebug &os, Tool t) {
    switch (t) {
    case Tool::hand:
        os << "hand";
        break;
    case Tool::draw_point:
        os << "draw_point";
        break;
    case Tool::draw_line:
        os << "draw_line";
        break;
    case Tool::select:
        os << "select";
        break;
    case Tool::move:
        os << "move";
        break;
    default:
        os << "<tool:unknown>";
        break;
    }
    return os;
}

QDebug &operator<<(QDebug &os, Point p) {
    os << p.x << ", " << p.y;
    return os;
}
