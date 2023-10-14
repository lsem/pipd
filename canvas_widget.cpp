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

    // if (m_state == CanvasState::idle || m_state == CanvasState::drawing) {
    //     m_state = CanvasState::drawing;
    //     m_current_shape.emplace_back(Point{.x = event->x(), .y = event->y()});
    //     this->update();
    // }
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

    if (m_selected_tool == Tool::draw) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        if (m_hand_tool_state == HandToolState::idle) {
            m_scale += (event->pixelDelta().y() / 120.0) / 10.0;
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
