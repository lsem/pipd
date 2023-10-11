#include "canvas.hpp"
#include <cstdlib>

#include <QPainter>
#include <QPaintEvent>

Canvas::Canvas(QWidget *parent) : QWidget(parent) {}

void Canvas::paintEvent(QPaintEvent *event) /*override*/ {
    QPainter painter;
    painter.begin(this);
    QBrush brush {QColor{64,32,64}};
    painter.fillRect(event->rect(), brush);
    painter.end();
    printf("ERROR: Canvas::paintEvent not implemented\n");
     //std::abort();

}
// how to clear settings on OTA?
// what about a case when we penalized something but after reset we use it again before feature can penalize it again.
