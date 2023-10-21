#include "canvas_widget.hpp"

#include "math.hpp"
#include "v2.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <vector>

namespace {
// QPointF to_qpointf(Point p) { return QPointF(p.x, p.y); }
// Point to_point(QPointF p) { return Point(p.x(), p.y()); }

Rect select_bbox(Point p, double size) {
    return Rect{.x = p.x - size / 2, .y = p.y - size / 2, .width = size, .height = size};
}

QRectF to_qrectf(Rect r) { return QRectF(r.x, r.y, r.width, r.height); }

bool in_rect(int x, int y, Rect r) {
    return (x >= r.x && x <= (r.x + r.width)) && (y > r.y && y <= (r.y + r.height));
}

bool in_rect(Point p, Rect r) { return in_rect(p.x, p.y, r); }

const double SELECT_TOOL_HIT_BBOX = 20.0;

std::array<Point, 4> line_bbox(Line l, double size) {

    std::array<Point, 4> ret;

    auto v = v2(l.a, l.b);
    auto perp_v = normalized(v2(-v.y, v.x)) * size / 2;
    ret[0] = l.a + perp_v;
    ret[1] = l.a + perp_v * -1;
    ret[2] = l.b + perp_v * -1;
    ret[3] = l.b + perp_v;

    return ret;
}

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

    // By default only select tool has tracking on.
    if (m_selected_tool == Tool::select) {
        setMouseTracking(true);
    } else {
        setMouseTracking(false);
    }
}

void CanvasWidget::paintEvent(QPaintEvent *event) /*override*/ {
    QPainter painter;

    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    render_background(&painter, event);

    painter.setTransform(get_transformation_matrix());

    render_lines(&painter, event);
    render_handles(&painter, event);
    render_debug_elements(&painter, event);

    painter.end();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    auto mouse_screen = Point{static_cast<double>(event->x()), static_cast<double>(event->y())};
    auto mouse_world = screen_to_world(mouse_screen);
    if (m_selected_tool == Tool::draw_point) {
        // ..
    } else if (m_selected_tool == Tool::draw_line) {
        if (m_draw_line_state == DrawLineState::point_a_placed) {
            m_line_point_b = mouse_world;
            update();
        }
    } else if (m_selected_tool == Tool::hand) {
        if (m_hand_tool_state == HandToolState::pressed) {
            auto dx = event->x() - m_prev_x;
            auto dy = event->y() - m_prev_y;
            m_prev_x = event->x();
            m_prev_y = event->y();

            m_translate_x -= dx;
            m_translate_y -= dy;

            qDebug() << "translate: " << m_translate_x << ", " << m_translate_y;
        }
        update();
    } else if (m_selected_tool == Tool::select) {
        m_hitting_line_id = false;
        for (auto &[line, id] : m_lines) {
            auto p = math::closest_point_to_line(line.a, line.b, mouse_world);
            if (len(v2(mouse_world, p)) < 10) {
                m_hitting_line_id = id;
            }
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
            setMouseTracking(false);
            update();
        } else {
            m_draw_line_state = DrawLineState::point_a_placed;
            m_line_point_a = mouse_world;
            setMouseTracking(true);
            qDebug() << "LINE: point A placed";
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

        m_projection_points.clear();
        for (auto &line : m_lines) {
            auto R = math::closest_point_to_line(line.l.a, line.l.b, mouse_world);
            m_projection_points.emplace_back(R);
            qDebug() << "added closest point: " << R.x << ", " << R.y;

            auto line_screen = world_to_screen(line.l);
            auto bbox_rect_pts = line_bbox(line_screen, 20.0);
            if (math::rect_point_hit_test(bbox_rect_pts, mouse_screen)) {
                qDebug() << "hit into line " << line.id.c_str() << "!!!!!";
            } else {
            }
        }
        update();
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

void draw_colored_line(QPainter *painter, QPointF p1, QPointF p2, QColor c) {
    QPen pen;
    pen.setColor(c);
    pen.setWidthF(1.0);
    painter->setPen(pen);
    painter->drawLine(p1, p2);
}

void draw_colored_point(QPainter *painter, Point p, QColor c) {
    QBrush point_brush{c};
    const size_t half_size = 10;
    QRectF point_rect{p.x - half_size, p.y - half_size, half_size * 2, half_size * 2};
    painter->fillRect(point_rect, point_brush);
}

void CanvasWidget::render_debug_elements(QPainter *painter, QPaintEvent *) {
    // Render bounding box for lines
    for (auto &[line, id] : m_lines) {
        auto &[a, b] = line;

        double size = 20.0 / m_scale;

        auto [r1, r2, r3, r4] = line_bbox(line, size);
        draw_colored_line(painter, r1, r2, QColor(100, 100, 100));
        draw_colored_line(painter, r2, r3, QColor(100, 100, 100));
        draw_colored_line(painter, r3, r4, QColor(100, 100, 100));
        draw_colored_line(painter, r4, r1, QColor(100, 100, 100));

        draw_colored_line(painter, a, r1, QColor(255, 0, 0));
        draw_colored_line(painter, a, r2, QColor(255, 255, 0));
        draw_colored_line(painter, b, r3, QColor(255, 0, 255));
        draw_colored_line(painter, b, r4, QColor(0, 255, 0));
    }

    for (auto p : m_projection_points) {
        draw_colored_point(painter, p, QColor(100, 100, 100));
    }

    if (!m_hitting_line_id.empty()) {
        auto it = std::find_if(m_lines.begin(), m_lines.end(),
                               [&](auto &l) { return l.id == m_hitting_line_id; });
        if (it != m_lines.end()) {
            draw_colored_line(painter, it->l.a, it->l.b, QColor(255, 0, 0));
        }
    }
}

void CanvasWidget::render_lines(QPainter *painter, QPaintEvent *) {
    for (auto &[line, id] : m_lines) {
        auto &[a, b] = line;
        QPen pen;
        pen.setColor(QColor{0, 0, 0});
        pen.setWidthF(1.0);
        painter->setPen(pen);
        painter->drawLine(a, b);
    }

    // TODO: move to separate renderer
    if (m_selected_tool == Tool::draw_line) {
        if (m_draw_line_state == DrawLineState::point_a_placed) {
            QPen pen;
            pen.setColor(QColor{100, 100, 100});
            pen.setWidthF(0.8 / m_scale);
            painter->setPen(pen);
            painter->drawLine(m_line_point_a.x, m_line_point_a.y, m_line_point_b.x,
                              m_line_point_b.y);
        }
    }
}

Point CanvasWidget::world_to_screen(Point p) { return p * get_transformation_matrix(); }

Point CanvasWidget::screen_to_world(Point p) { return p * get_transformation_matrix().inverted(); }

Line CanvasWidget::world_to_screen(Line p) {
    return Line{.a = world_to_screen(p.a), .b = world_to_screen(p.b)};
}
Line CanvasWidget::screen_to_world(Line p) {
    return Line{.a = screen_to_world(p.a), .b = screen_to_world(p.b)};
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
    double cx = width() / 2;
    double cy = height() / 2;
    m.translate(cx, cy);
    m.scale(m_scale, m_scale);
    m.translate(-cx, -cy);
    m.translate(-m_translate_x, -m_translate_y);
    return m;
}

// NOW:
//     selected line has two handles. we can move point of one line wherever we want.
//     but even more nemeficial is to create a point.
//     we can also move line itself which will translate the line accordingly.
//    Once we have this basic functionality implemented, we can try to draw something
//    non-trivial. For this we will need to be able to set widths. We will also need to be able
//    to resize things according to specified size. We will also nened to remove things.
