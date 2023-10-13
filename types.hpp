#pragma once
#include <QDebug>

enum class Tool { hand, draw };

QDebug &operator<<(QDebug &os, Tool t);

struct Point {
    int x;
    int y;
};

QDebug &operator<<(QDebug &os, Point t);
