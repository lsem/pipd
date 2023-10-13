#include "canvas_widget.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <vector>

struct Point {
    int x;
    int y;
};

enum class CanvasState { idle, drawing };

struct CanvasWidget::StateData {
    CanvasState state = CanvasState::idle;
    std::vector<Point> current_shape;
};

// Represents a model of what we are editing/displaying.
struct Model {};

// Basically, our renderer does not own a model but draws it. Can have its own
// state, e.g. some cache or something.
class Renderer {
  public:
    void render_background(QPainter *painter, QPaintEvent *event) {
        QBrush brush{QColor{227, 227, 227}};
        painter->fillRect(event->rect(), brush);
    }

    void render_currently_drawing(QPainter *painter, CanvasWidget::StateData &state_data) {
        const auto &current_shape = state_data.current_shape;
        // render handles
        qDebug() << "current shape size: " << current_shape.size();
        for (auto &p : current_shape) {
            const int size = 20;
            const auto half_size = size / 2;
            QRect point_rect{p.x - half_size, p.y - half_size, size, size};
            QBrush point_brush{QColor{255, 0, 0}};
            qDebug() << "point.rect: " << point_rect;
            painter->fillRect(point_rect, point_brush);
        }

        // render lines
        if (current_shape.size() > 1) {
            QPen pen;
            pen.setColor(QColor{0, 0, 0});
            pen.setWidth(4);

            for (size_t i = 1; i < current_shape.size(); ++i) {
                auto &p1 = current_shape[i - 1];
                auto &p2 = current_shape[i];
                painter->drawLine(p1.x, p1.y, p2.x, p2.y);
            }
        }
    }

    void render(QPaintEvent *event, QPainter *painter, CanvasWidget::StateData &state_data) {
        render_background(painter, event);
        render_currently_drawing(painter, state_data);
    }
};

CanvasWidget::CanvasWidget(QWidget *parent)
    // Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
    : QWidget(parent), m_renderer(std::make_unique<Renderer>()), m_model(std::make_unique<Model>()),
      m_state_data(std::make_unique<StateData>()) {}

CanvasWidget::~CanvasWidget() = default;

void CanvasWidget::select_tool(Tool tool) { qDebug() << "tool selected: " << tool; }

void CanvasWidget::paintEvent(QPaintEvent *event) /*override*/ {
    QPainter painter;
    painter.begin(this);
    m_renderer->render(event, &painter, *m_state_data);
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
