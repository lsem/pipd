#include "types.hpp"

QDebug &operator<<(QDebug &os, Tool t) {
    switch (t) {
    case Tool::hand:
        os << "hand";
        break;
    case Tool::draw:
        os << "draw";
        break;
    default:
        os << "<tool:unknown>";
        break;
    }
    return os;
}
