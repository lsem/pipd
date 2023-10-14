#include "canvas_widget.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <vector>

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    // put few points for the test.
    m_current_shape.emplace_back(Point{.x = 20, .y = 20});
    m_current_shape.emplace_back(Point{.x = 100, .y = 100});
}

CanvasWidget::~CanvasWidget() = default;

void CanvasWidget::select_tool(Tool tool) {
    qDebug() << "tool selected: " << tool;
    m_selected_tool = tool;
}

void CanvasWidget::paintEvent(QPaintEvent *event) /*override*/ {
    QPainter painter;
    painter.begin(this);
    render_background(&painter, event);
    painter.translate(m_translate_x, m_translate_y);
    painter.scale(m_scale, m_scale);
    render_lines(&painter, event);
    render_handles(&painter, event);
    painter.end();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_selected_tool == Tool::draw) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        if (m_hand_tool_state == HandToolState::pressed) {
            auto dx = event->x() - m_prev_x;
            auto dy = event->y() - m_prev_y;
            m_prev_x = event->x();
            m_prev_y = event->y();

            m_translate_x += dx;
            m_translate_y += dy;
        }
        update();
    }
}

// what is origin in our scene?
// It can be just 0. Then if we want to have coordinates in real world (GPS) we can fit this origin
// to some coorinates and then find coorindates of all objects on the scene. Just in case. But, for
// now we can assume that we have origin and all coordinates are translated in centimeters to this
// origin. When we start drawing, position 0,0 corresponds to origin of World. So putting object 0.0
// puts object translated to 0cm to the origin.
// Then, when we zoom to 2x. 
//

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    qDebug() << "clicked at " << event->x() << ", " << event->y();

    if (m_selected_tool == Tool::draw) {
        // draw tool is for drawing things
    } else if (m_selected_tool == Tool::hand) {
        // hand tool is for camera control
        if (m_hand_tool_state == HandToolState::idle) {
            m_hand_tool_state = HandToolState::pressed;
            m_prev_x = event->x();
            m_prev_y = event->y();
        }
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_selected_tool == Tool::draw) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        // hand tool is for camera control.
        if (m_hand_tool_state == HandToolState::pressed) {
            m_hand_tool_state = HandToolState::idle;
        }
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    // qDebug() << "pixelDelta.x: " << event->pixelDelta().x();
    // qDebug() << "pixelDelta.y: " << event->pixelDelta().y();
    qDebug() << "angle_delta.x: " << event->angleDelta().x();
    qDebug() << "angle_delta.y: " << event->angleDelta().y();

    // Most mouse types work in increments of 15.0 degress but Qt returns eights of degree.
    // We assume that 15 degrees will correspond to 0.1 scale so minimal increment of wheel results
    // in + 0.1 or -0.1 zoom.

    const auto delta_degress = event->angleDelta().y() / 8;
    const auto delta_zoom = (delta_degress / 15.0) / 10.0;

    if (m_selected_tool == Tool::draw) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        if (m_hand_tool_state == HandToolState::idle) {
            m_scale += delta_zoom;
            qDebug() << "m_zoom: " << m_scale;
            update();
        }
    }
}

void CanvasWidget::render_background(QPainter *painter, QPaintEvent *event) {
    QBrush brush{QColor{227, 227, 227}};
    painter->fillRect(event->rect(), brush);
}

void CanvasWidget::render_handles(QPainter *painter, QPaintEvent *) {
    for (auto &p : m_current_shape) {
        const double size = 10 / m_scale;
        const auto half_size = size / 2;

        QRectF point_rect{p.x - half_size, p.y - half_size, size, size};
        QBrush point_brush{QColor{255, 0, 0}};
        // qDebug() << "point.rect: " << point_rect;
        painter->fillRect(point_rect, point_brush);
    }
}

void CanvasWidget::render_lines(QPainter *painter, QPaintEvent *) {
    if (m_current_shape.size() > 1) {
        QPen pen;
        pen.setColor(QColor{0, 0, 0});
        pen.setWidthF(1.5 / m_scale);
        painter->setPen(pen);

        for (size_t i = 1; i < m_current_shape.size(); ++i) {
            auto &p1 = m_current_shape[i - 1];
            auto &p2 = m_current_shape[i];
            painter->drawLine(p1.x, p1.y, p2.x, p2.y);
        }
    }
}
