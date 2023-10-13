#include "canvas_widget.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <vector>

enum class CanvasState { idle, drawing };

struct CanvasWidget::StateData {
    CanvasState state = CanvasState::idle;
    std::vector<Point> current_shape;
};

CanvasWidget::StateData &CanvasWidget::data() const { return *m_state_data; }
CanvasWidget::StateData &CanvasWidget::data() { return *m_state_data; }
CanvasWidget::CanvasWidget(QWidget *parent)
    : QWidget(parent), m_state_data(std::make_unique<StateData>()) {}

CanvasWidget::~CanvasWidget() = default;

void CanvasWidget::select_tool(Tool tool) { qDebug() << "tool selected: " << tool; }

void CanvasWidget::paintEvent(QPaintEvent *event) /*override*/ {
    QPainter painter;
    painter.begin(this);
    render_background(&painter, event);
    render_lines(&painter, event);
    render_handles(&painter, event);
    painter.end();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    // so we clicked somewhere..
    qDebug() << "clicked at " << event->x() << ", " << event->y();

    if (m_state_data->state == CanvasState::idle || m_state_data->state == CanvasState::drawing) {
        m_state_data->state = CanvasState::drawing;
        m_state_data->current_shape.emplace_back(Point{.x = event->x(), .y = event->y()});
        this->update();
    }

    // depending on currenty selected tool we have some handling.
    // for the moment, lets assume currently selected tool is D (Drawwing).
    //     if in idle, then click puts a point and transitions to state,
    //     continue-drawing.
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {}
void CanvasWidget::moveEvent(QMoveEvent *event) {}
void CanvasWidget::wheelEvent(QWheelEvent *event) {}

void CanvasWidget::render_background(QPainter *painter, QPaintEvent *event) {
    QBrush brush{QColor{227, 227, 227}};
    painter->fillRect(event->rect(), brush);
}
void CanvasWidget::render_handles(QPainter *painter, QPaintEvent *) {
    for (auto &p : data().current_shape) {
        const int size = 20;
        const auto half_size = size / 2;
        QRect point_rect{p.x - half_size, p.y - half_size, size, size};
        QBrush point_brush{QColor{255, 0, 0}};
        qDebug() << "point.rect: " << point_rect;
        painter->fillRect(point_rect, point_brush);
    }
}
void CanvasWidget::render_lines(QPainter *painter, QPaintEvent *) {
    auto &current_shape = data().current_shape;
    if (current_shape.size() > 1) {
        QPen pen;
        pen.setColor(QColor{0, 0, 0});
        pen.setWidth(4);

        for (size_t i = 1; i < data().current_shape.size(); ++i) {
            auto &p1 = data().current_shape[i - 1];
            auto &p2 = current_shape[i];
            painter->drawLine(p1.x, p1.y, p2.x, p2.y);
        }
    }
}
