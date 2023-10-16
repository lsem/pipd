#include "canvas_widget.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <vector>

namespace {
QPointF to_qpointf(Point p) { return QPointF(p.x, p.y); }
Point to_point(QPointF p) { return Point(p.x(), p.y()); }

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
    painter.setRenderHint(QPainter::Antialiasing);

    render_background(&painter, event);

    painter.setTransform(get_transformation_matrix());

    render_lines(&painter, event);
    render_handles(&painter, event);
    painter.end();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_selected_tool == Tool::draw_point) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        if (m_hand_tool_state == HandToolState::pressed) {
            auto dx = event->x() - m_prev_x;
            auto dy = event->y() - m_prev_y;
            m_prev_x = event->x();
            m_prev_y = event->y();

            m_translate_x -= dx;
            m_translate_y -= dy;

            qDebug() << "translate: " << m_translate_x << ", " << m_translate_y;
        } else {
            assert(false && "what!");
        }
        update();
    }
}

std::string random_id() {
    std::string s;
    for (int i = 0; i < 12; ++i) {
        s += (rand() % 'Z' - 'A') + 'A';
    }
    return s;
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {

    double x = event->x();
    double y = event->y();

    qDebug() << "clicked at " << x << ", " << y;

    auto mouse_screen = Point{x, y};
    auto mouse_world = screen_to_world(mouse_screen);

    if (m_selected_tool == Tool::draw_point) {

        qDebug() << "new point at: " << mouse_world;

        // draw tool is for drawing things
        m_points.emplace_back(mouse_world, random_id());

        update();

    } else if (m_selected_tool == Tool::draw_line) {
        if (m_draw_line_state == DrawLineState::point_a_placed) {
            qDebug() << "point A was placed";
            LineObj new_line;
            new_line.id = random_id();
            new_line.l.a = m_line_point_a;
            new_line.l.b = mouse_world;
            m_lines.emplace_back(new_line);
            m_draw_line_state = DrawLineState::waiting_point_a;
            update();
        } else {
            m_draw_line_state = DrawLineState::point_a_placed;
            m_line_point_a = mouse_world;
            qDebug() << "point A placed";
        }
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
            auto point_screen = world_to_screen(p);
            if (in_rect(mouse_screen, select_bbox(point_screen, SELECT_TOOL_HIT_BBOX))) {
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
    if (m_selected_tool == Tool::draw_point) {
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
    const auto delta_zoom = (delta_degress / 15.0) / 10.0; // mapped to -1/+1 for mouse mouses

    const auto x = event->position().x();
    const auto y = event->position().y();
    // const auto x = width()/2.0;
    // const auto y = height()/2.0;

    if (m_selected_tool == Tool::draw_point) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        //	m_hand_tool_state = HandleToolState::
        if (m_hand_tool_state == HandToolState::idle) {
            if (!m_zoom_center_opt) {
                m_zoom_center_opt = {x, y};
            }
            // const auto mouse_in_world_before = screen_to_world(*m_zoom_center_opt);
            m_scale += delta_zoom;
            // auto mouse_in_world_after = screen_to_world(*m_zoom_center_opt);
            // Point delta{mouse_in_world_after.x - mouse_in_world_before.x,
            //             mouse_in_world_after.y - mouse_in_world_before.y};
            // m_translate_x -= delta.x;
            // m_translate_y -= delta.y;
            //            qDebug() << "mouse in world delta: " << delta;
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
    for (auto &[line, id] : m_lines) {
        auto &[a, b] = line;
        QPen pen;
        pen.setColor(QColor{0, 0, 0});
        pen.setWidthF(1.0 / m_scale);
        painter->setPen(pen);
        painter->drawLine(a.x, a.y, b.x, b.y);
    }
}

Point CanvasWidget::world_to_screen(Point p) {
    return to_point(to_qpointf(p) * get_transformation_matrix());
}

Point CanvasWidget::screen_to_world(Point p) {
    return to_point(to_qpointf(p) * get_transformation_matrix().inverted());
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

QTransform CanvasWidget::get_transformation_matrix() const {
    QTransform m;
    m.translate(-m_translate_x, -m_translate_y);
    m.scale(m_scale, m_scale);
    //    m.translate(width() / 2.0, height() / 2.0);
    return m;
}
