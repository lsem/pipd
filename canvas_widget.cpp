#include "canvas_widget.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <vector>

namespace {
Rect select_bbox(Point p, double size) {
    return Rect{.x = p.x - size / 2, .y = p.y - size / 2, .width = size, .height = size};
}

QRectF to_qrectf(Rect r) { return QRectF(r.x, r.y, r.width, r.height); }

bool in_rect(int x, int y, Rect r) {
    return (x >= r.x && x <= (r.x + r.width)) && (y > r.y && y <= (r.y + r.height));
}

bool in_rect(Point p, Rect r) { return in_rect(p.x, p.y, r); }

const double SELECT_TOOL_HIT_BBOX = 20.0;

} // namespace

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
    // put few points for the test.
    m_points.emplace_back(PointObj{20, 20, "1"});
    m_points.emplace_back(PointObj{100, 100, "2"});
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
// next taks will be layers:
//   next major milestone will be ability to draw the floor on layer.
//   BTW, what is floor? it seems like it is something like a scane, like a separate image.
//   Theoretically, this floors can even be displayed on single UI side by side.
//   Each floor may have mulitple layers.
// Once we have floor, we can go to duct work and edit real world model.

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
    } else if (m_selected_tool == Tool::select) {
        // we are going to test for hits into either points or lines.
        // line is independent thing to point.
        for (auto &p : m_points) {
            auto unprojected_mouse_pos = unproject(Point{(double)event->x(), (double)event->y()});
            if (in_rect(unprojected_mouse_pos, select_bbox(p, SELECT_TOOL_HIT_BBOX / m_scale))) {
                if (!is_object_selected(p)) {
                    qDebug() << "hit into point!";
                    mark_object_selected(p);
                    update();
                } else {
                    unmark_object_selected(p);
                    update();
                }
            } else {
                qDebug("not hit!");
            }

            // we can select things. After things are selected they can be moved, we can display
            // properties of things.
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
    for (auto &p : m_points) {
        const double size = 10 / m_scale;
        const auto half_size = size / 2;

        QRectF point_rect{p.x - half_size, p.y - half_size, size, size};
        QBrush point_brush{QColor{255, 0, 0}};

        if (is_object_selected(p)) {
            point_brush.setColor(QColor(255, 255, 255));
        }
        // qDebug() << "point.rect: " << point_rect;
        painter->fillRect(point_rect, point_brush);

        // TODO: put in boolean (if m_debug_mode)
        QRectF hit_bbox = to_qrectf(select_bbox(p, SELECT_TOOL_HIT_BBOX / m_scale));
        QPen hit_bbox_pen{QColor{100, 100, 100}};
        hit_bbox_pen.setWidth(0.5 / m_scale);
        painter->setPen(hit_bbox_pen);
        painter->drawRect(hit_bbox);
    }
}

void CanvasWidget::render_lines(QPainter *painter, QPaintEvent *) {
    if (m_points.size() > 1) {
        QPen pen;
        pen.setColor(QColor{0, 0, 0});
        pen.setWidthF(1.5 / m_scale);
        painter->setPen(pen);

        for (size_t i = 1; i < m_points.size(); ++i) {
            auto &p1 = m_points[i - 1];
            auto &p2 = m_points[i];
            painter->drawLine(p1.x, p1.y, p2.x, p2.y);
        }
    }
}

// World to View transformation
Point CanvasWidget::project(Point p) {
    assert(false && "not implemented");
    return p;
}

// View to World transofrmation
Point CanvasWidget::unproject(Point p) {
    return Point{(p.x - m_translate_x) / m_scale, (p.y - m_translate_y) / m_scale};
}

void CanvasWidget::mark_object_selected(const PointObj &o) { select_object_by_id_impl(o.id); }

void CanvasWidget::mark_object_selected(const LineObj &o) { select_object_by_id_impl(o.id); }

void CanvasWidget::unmark_object_selected(const PointObj &o) { deselect_object_by_id_impl(o.id); }

void CanvasWidget::unmark_object_selected(const LineObj &o) { deselect_object_by_id_impl(o.id); }

void CanvasWidget::select_object_by_id_impl(const std::string &id) {
    m_selected_objects.emplace_back(id);
}

void CanvasWidget::deselect_object_by_id_impl(const std::string &id) {
    m_selected_objects.erase(std::remove(m_selected_objects.begin(), m_selected_objects.end(), id),
                             m_selected_objects.end());
}

bool CanvasWidget::is_object_selected(const PointObj &o) {
    return std::find(m_selected_objects.begin(), m_selected_objects.end(), o.id) !=
           m_selected_objects.end();
}

bool CanvasWidget::is_object_selected(const LineObj &o) {
    return std::find(m_selected_objects.begin(), m_selected_objects.end(), o.id) !=
           m_selected_objects.end();
}
