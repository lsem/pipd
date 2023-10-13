#pragma once
#include <QDebug>

enum class Tool { hand, draw };

QDebug &operator<<(QDebug &os, Tool t);
