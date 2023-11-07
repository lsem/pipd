#include "canvas_widget.hpp"

#include "math.hpp"
#include "v2.hpp"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

#include <sstream>
#include <vector>

namespace {
const auto Pink = QColor(255, 20, 147);
const auto Blue = QColor(66, 135, 245);
const auto Grey = QColor(100, 100, 100);
const auto LightGrey = QColor(200, 200, 200);

const auto HowerColor = Blue;

const int RULER_WIDTH_PIXELS = 10;

std::string format_distance_display_text(double distance) {
    std::stringstream ss;
    ss << static_cast<int>(std::round(distance)) << "m";
    return ss.str();
}

QPointF to_qpointf(Point p) { return QPointF(p.x, p.y); }
Point to_point(QPointF p) { return Point(p.x(), p.y()); }
QRectF to_qrectf(Rect r) { return QRectF(r.x, r.y, r.width, r.height); }

Rect select_bbox(Point p, double size) {
    return Rect{.x = p.x - size / 2, .y = p.y - size / 2, .width = size, .height = size};
}

bool in_rect(int x, int y, Rect r) {
    return (x >= r.x && x <= (r.x + r.width)) && (y > r.y && y <= (r.y + r.height));
}

bool in_rect(Point p, Rect r) { return in_rect(p.x, p.y, r); }

bool point_howers_line(Point p, Line l) {
    return len(v2{p, math::closest_point_to_line(l.a, l.b, p)}) < 10;
};

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
// Scales line around the center. Center is not changed but endpoints go nearer or further to the
// center.
Line scale_line(Line l, double factor) {
    v2 center = (l.a + l.b) / 2;
    QTransform m;
    m.translate(center.x, center.y);
    m.scale(factor, factor);
    m.translate(-center.x, -center.y);
    QPointF q1 = to_qpointf(l.a) * m;
    QPointF q2 = to_qpointf(l.b) * m;
    return Line{to_point(q1), to_point(q2)};
}
Line rotate_line(Line l, double angle_rad) {
    v2 center = (l.a + l.b) / 2;
    QTransform m;
    m.translate(center.x, center.y);
    m.rotateRadians(angle_rad);
    m.translate(-center.x, -center.y);
    QPointF q1 = to_qpointf(l.a) * m;
    QPointF q2 = to_qpointf(l.b) * m;
    return Line{to_point(q1), to_point(q2)};
}

void draw_colored_line(QPainter *painter, Point p1, Point p2, QColor c, double width = 1.0) {
    QPen pen;
    pen.setColor(c);
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->drawLine(to_qpointf(p1), to_qpointf(p2));
}

void draw_rect(QPainter *painter, Rect r, QColor c, double width = 1.0) {
    QPen pen;
    pen.setColor(c);
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->drawRect(to_qrectf(r));
}

void draw_colored_line(QPainter *painter, Line l, QColor c, double width = 1.0) {
    draw_colored_line(painter, l.a, l.b, c, width);
}

void draw_dashed_line(QPainter *painter, Point p1, Point p2, QColor c, double width = 1.0) {
    QPen pen;
    pen.setColor(c);
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->drawLine(to_qpointf(p1), to_qpointf(p2));
}

void draw_dashed_line(QPainter *painter, Line l, QColor c, double width = 1.0) {
    draw_dashed_line(painter, l.a, l.b, c, width);
}

void draw_colored_point(QPainter *painter, Point p, QColor c, double size = 5.0) {
    QBrush point_brush{c};
    const size_t half_size = size / 2;
    QRectF point_rect{p.x - half_size, p.y - half_size, size, size};
    painter->fillRect(point_rect, point_brush);
}

} // namespace

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent), m_move_tool(*this, m_model) {
    // put few points for the test.
    // m_model.points.emplace_back(PointObj{{20, 20}, "1"});
    // m_model.points.emplace_back(PointObj{{100, 100}, "2"});
}

CanvasWidget::~CanvasWidget() = default;

void CanvasWidget::select_tool(Tool tool) {
    qDebug() << "tool selected: " << tool;
    m_selected_tool = tool;

    // By default only select tool has tracking one
    if (m_selected_tool == Tool::select || m_selected_tool == Tool::move) {
        setMouseTracking(true);
    } else {
        setMouseTracking(false);
    }

    // Amount of rendering can depend on a tool so after activation update needed.
    update();
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

    if (m_selected_tool == Tool::guide) {
        render_rulers(&painter, event);
    }

    render_guides(&painter, event);

    render_rects(&painter, event);

    painter.end();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
    auto mouse_screen = Point{static_cast<double>(event->x()), static_cast<double>(event->y())};
    auto mouse_world = screen_to_world(mouse_screen);
    double x = event->x();
    double y = event->y();
    auto dx = x - m_prev_x;
    auto dy = y - m_prev_y;
    m_prev_x = event->x();
    m_prev_y = event->y();
    double sdx = dx / m_scale;
    double sdy = dy / m_scale;

    switch (m_selected_tool) {
    case Tool::draw_point: {
        break;
    }
    case Tool::draw_line: {
        if (m_draw_line_state == DrawLineState::point_a_placed) {
            m_line_point_b = mouse_world;
            update();
        }

        break;
    }
    case Tool::hand: {
        if (m_hand_tool_state == HandToolState::pressed) {
            m_translate_x -= dx;
            m_translate_y -= dy;

            qDebug() << "translate: " << m_translate_x << ", " << m_translate_y;
        }
        update();
        break;
    }
    case Tool::select: {
        m_hitting_line_id = false;
        for (auto &line_obj : m_model.lines) {
            auto &line = line_obj.l;
            auto &id = line_obj.id;

            auto p = math::closest_point_to_line(line.a, line.b, mouse_world);
            if (len(v2(mouse_world, p)) < 10) {
                m_hitting_line_id = id;
            }
        }
        update();
        break;
    }
    case Tool::move: {
        bool update_needed = false;

        for (auto &line : m_model.lines) {
            if (line.flags & ObjFlags::moving) {
                update_needed = true;
                line.shadow_l.a.x += sdx;
                line.shadow_l.b.x += sdx;
                line.shadow_l.a.y += sdy;
                line.shadow_l.b.y += sdy;
            } else if (line.flags & ObjFlags::a_endpoint_move) {
                qDebug() << "A endpoint is being moved";
                update_needed = true;
                line.shadow_l.a.x += sdx;
                line.shadow_l.a.y += sdy;
            } else if (line.flags & ObjFlags::b_endpoint_move) {
                qDebug() << "B endpoint is being moved";
                update_needed = true;
                line.shadow_l.b.x += sdx;
                line.shadow_l.b.y += sdy;
            } else {
                // Clear all hower-related flag to make transitions between howered object correct.
                // E.g. when line is howered and then we hower endpoint line should lose its
                // howerness.
                line.flags &= ~(ObjFlags::howered | ObjFlags::a_endpoint_move_howered |
                                ObjFlags::b_endpoint_move_howered);
                update_needed = true;

                auto &line_geometry = line.l;
                if (math::points_distance(line_geometry.a, mouse_world) < 10.0) {
                    qDebug() << "MOVE: around A endpoint";
                    line.flags |= ObjFlags::a_endpoint_move_howered;
                    update_needed = true;
                } else if (math::points_distance(line_geometry.b, mouse_world) < 10.0) {
                    qDebug() << "MOVE: around B endpoint";
                    line.flags |= ObjFlags::b_endpoint_move_howered;

                    update_needed = true;
                } else if (auto dist = len(
                               v2{mouse_world, math::closest_point_to_line(
                                                   line_geometry.a, line_geometry.b, mouse_world)});
                           dist < 10) {
                    qDebug() << "MOVE: around line " << line.id.c_str();
                    line.flags |= ObjFlags::howered;
                    update_needed = true;
                }
            }
        }

        for (auto &rect : m_model.rects) {
            auto flags_before = rect.flags;
            // TODO: Current Move tool is basically resize tool. Instead, we should have separate
            // tool that would move entire object: line or rect. and separate tool for resize: which
            // allows to change only size of an on object.

            if (rect.flags & ObjFlags::top_rect_line_move) {
                // TODO: here we should have a command instead of direct model manipulation.
                rect.shadow_rect.move_top_line(sdy);
            } else if (rect.flags & ObjFlags::bottom_rect_line_move) {
                rect.shadow_rect.move_bottom_line(sdy);
            } else if (rect.flags & ObjFlags::left_rect_line_move) {
                rect.shadow_rect.move_left_line(sdx);
            } else if (rect.flags & ObjFlags::right_rect_line_move) {
                rect.shadow_rect.move_right_line(sdx);
            } else {

                // before testing new hovers, first clean all existing hovers to handle when line is
                // no longer hovered
                rect.flags &= ~(
                    ObjFlags::top_rect_line_move_howered | ObjFlags::bottom_rect_line_move_howered |
                    ObjFlags::left_rect_line_move_howered | ObjFlags::right_rect_line_move_howered);

                auto &geometry = rect.rect;
                if (point_howers_line(mouse_world, geometry.top_line())) {
                    rect.flags |= ObjFlags::top_rect_line_move_howered;
                } else if (point_howers_line(mouse_world, geometry.bottom_line())) {
                    rect.flags |= ObjFlags::bottom_rect_line_move_howered;
                } else if (point_howers_line(mouse_world, geometry.left_line())) {
                    rect.flags |= ObjFlags::left_rect_line_move_howered;
                } else if (point_howers_line(mouse_world, geometry.right_line())) {
                    rect.flags |= ObjFlags::right_rect_line_move_howered;
                }
            }

            //            if (flags_before != rect.flags) {
            update_needed = true;
            //            }
        }
        update_needed = true;
        if (update_needed)
            update();

        break;
    }
    case Tool::guide: {
        qDebug() << "GUIDE: MOVE: " << x << ", " << y;

        // How we are supposed to handle mouse move for guide?
        // guide is always sliding along perpendicular line to the origin (anchor) line
        if (!m_guide_tool_state.guide_active) {
            return;
        }

        // Create a line of width 1.0 in a world, just for the sake of angle.
        auto &anchor_line = m_guide_tool_state.anchor_line;
        v2 a{anchor_line.a, anchor_line.b};

        Point p1{mouse_world.x - 0.5, mouse_world.y};
        Point p2{mouse_world.x + 0.6, mouse_world.y};

        auto alpha = math::angle_between_vectors(a, v2{1.0, 0.0});

        auto l1 = scale_line(Line(p1, p2), 100'000);
        l1 = rotate_line(l1, -alpha);

        m_guide_tool_state.guide_line = l1;

        update();
        break;
    }
    case Tool::rectangle: {
        qDebug() << "MMOVE: RECT: " << x << ", " << y;
        // draw rectable from start point to current point
        if (m_rect_tool_state.rect_active) {
            m_rect_tool_state.p2 = mouse_world;
            update();
        } else {
            // .. handling hovers and snapping
        }
        // m_rect_tool_state.start_pos = mouse_world;
        // m_rect_tool_state.rect_active = true;
        break;
    }
    default:
        break;
    }
}

std::string random_id() {
    std::string s;
    for (int i = 0; i < 12; ++i) {
        s += (rand() % ('Z' - 'A')) + 'A';
    }
    return s;
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
    double x = event->x();
    double y = event->y();

    m_prev_x = x;
    m_prev_y = y;

    qDebug() << "clicked at " << x << ", " << y;

    auto mouse_screen = Point{x, y};
    auto mouse_world = screen_to_world(mouse_screen);

    switch (m_selected_tool) {
    case Tool::draw_point: {
        qDebug() << "new point at: " << mouse_world;

        // draw tool is for drawing things
        m_model.points.emplace_back(mouse_world, random_id());

        update();
        break;
    }
    case Tool::draw_line:
        if (m_draw_line_state == DrawLineState::point_a_placed) {
            qDebug() << "point A was placed";
            LineObj new_line;
            new_line.id = random_id();
            new_line.l.a = m_line_point_a;
            new_line.l.b = mouse_world;
            m_model.lines.emplace_back(new_line);
            // m_model.points.emplace_back(new_line.l.a, new_line.id + "__A");
            // m_model.points.emplace_back(new_line.l.b, new_line.id + "__B");
            m_draw_line_state = DrawLineState::waiting_point_a;

            setMouseTracking(false);
            update();
            break;
        } else {

            m_draw_line_state = DrawLineState::point_a_placed;
            m_line_point_a = mouse_world;
            setMouseTracking(true);
            qDebug() << "LINE: point A placed";
            break;
        }
    case Tool::hand: {
        // hand tool is for camera control
        if (m_hand_tool_state == HandToolState::idle) {
            m_hand_tool_state = HandToolState::pressed;
            m_prev_x = event->x();
            m_prev_y = event->y();
        }
        break;
    }
    case Tool::select: {
        // we are going to test for hits into either points or lines.
        // line is independent thing to point.
        for (auto &p : m_model.points) {
            auto point_screen = world_to_screen(p.pt);
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
        for (auto &line : m_model.lines) {
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
        break;
    }
    case Tool::move: {
        // By default qt enables tracking only while button is pressed but we want to have
        // object moved without any press.
        setMouseTracking(true);
        qDebug() << "mousePress: move tool selected";
        // Selecto tool works only on objects that are currently under cursor.
        // so we first need to find what object is under cursor. We expect there could be only
        // one object under cursor. If this is not possible then we can use preselected tool
        // with Select tool and by doing this at the moment we try to apply Move tool, we should
        // already have object selected so we can ignore all objects which are not in selected
        // state. But for for now, for the sake of simplicity, we can just consider everything
        // and see how it works.

        for (auto &line : m_model.lines) {
            if (line.flags & ObjFlags::moving) {
                // End of line move
                line.flags &= ~ObjFlags::moving;
                line.l = line.shadow_l;
            } else if (line.flags & (ObjFlags::a_endpoint_move | ObjFlags::b_endpoint_move)) {
                // Enf of line endpoint move
                line.flags &= ~(ObjFlags::a_endpoint_move | ObjFlags::b_endpoint_move);
                line.l = line.shadow_l;
            } else {
                // Beginning of linne/endpoints move
                auto &line_geometry = line.l;

                // Move tool has different handling of lines and endpoints. For endpoints, Move
                // tool moves endpoints and lines follow them if they are associated with it.
                // For line it translates the line as is.
                // TODO: in mouseMove handler, highlight current endpoints or line that. We can
                // either repeat this logic or we can somehow delegate this into Move tool. What
                // if move tool can handle events. class MoveTool { void on_mouse_move(); void
                // on_press(); void on_release(); void render_state(QPainter, Model) {} }
                if (math::points_distance(line_geometry.a, mouse_world) < 10.0) {
                    qDebug() << "MOVE: around A endpoint";
                    line.flags = ObjFlags::a_endpoint_move;
                    line.shadow_l = line.l;
                } else if (math::points_distance(line_geometry.b, mouse_world) < 10.0) {
                    qDebug() << "MOVE: around B endpoint";
                    line.flags = ObjFlags::b_endpoint_move;
                    line.shadow_l = line.l;
                } else {
                    qDebug() << "around line";
                    auto r =
                        math::closest_point_to_line(line_geometry.a, line_geometry.b, mouse_world);
                    const double dist = len(v2{mouse_world, r});
                    if (dist < 10) {
                        qDebug() << "The line [" << line.id.c_str() << "] is close to cursor";
                        line.flags |= ObjFlags::moving;
                        line.shadow_l = line.l;
                    }
                }
            }
            update();
        } // lines move

        for (auto &rect : m_model.rects) {
            auto &geometry = rect.rect;

            if (rect.flags & ObjFlags::top_rect_line_move) {
                rect.rect = rect.shadow_rect;
                rect.flags &= ~ObjFlags::top_rect_line_move;
            } else if (rect.flags & ObjFlags::bottom_rect_line_move) {
                rect.rect = rect.shadow_rect;
                rect.flags &= ~ObjFlags::bottom_rect_line_move;
            } else if (rect.flags & ObjFlags::left_rect_line_move) {
                rect.rect = rect.shadow_rect;
                rect.flags &= ~ObjFlags::left_rect_line_move;
            } else if (rect.flags & ObjFlags::right_rect_line_move) {
                rect.rect = rect.shadow_rect;
                rect.flags &= ~ObjFlags::right_rect_line_move;
            } else {
                if (point_howers_line(mouse_world, geometry.top_line())) {
                    rect.flags |= ObjFlags::top_rect_line_move;
                    rect.shadow_rect = rect.rect;
                } else if (point_howers_line(mouse_world, geometry.bottom_line())) {
                    rect.flags |= ObjFlags::bottom_rect_line_move;
                    rect.shadow_rect = rect.rect;
                } else if (point_howers_line(mouse_world, geometry.left_line())) {
                    rect.flags = ObjFlags::left_rect_line_move;
                    rect.shadow_rect = rect.rect;
                } else if (point_howers_line(mouse_world, geometry.right_line())) {
                    rect.flags = ObjFlags::right_rect_line_move;
                    rect.shadow_rect = rect.rect;
                }
            }
        }
        update(); // TODO: check if any flag changed.

        break;
    }
    case Tool::guide: {
        // guide starts from somewhere and stops somewhere. Orientation depends on baseline
        // where it started from. e.g. if we started from ruller/left side then we are going to
        // create vertical line. if we started from another line/edge, then angle will ba taken
        // from there.
        //
        // so check if we have some line/edge/anthing that we can take angle from.

        // FIXME: do it more selectively
        setMouseTracking(true);

        // Existing lines
        for (auto &line : m_model.lines) {
            auto &line_geometry = line.l;
            auto r = math::closest_point_to_line(line_geometry.a, line_geometry.b, mouse_world);
            const double dist = len(v2{mouse_world, r});
            if (dist < 10) {
                qDebug() << "GUIDE: hit into line " << line.id.c_str();
                m_guide_tool_state.guide_active = true;
                m_guide_tool_state.anchor_line = line_geometry;
                m_guide_tool_state.guide_line = m_guide_tool_state.anchor_line;

                update();
                return;
            }
        }

        // Rulers
        if (mouse_screen.x > (width() - RULER_WIDTH_PIXELS)) {
            qDebug() << "On vertical rule (right)";
            m_guide_tool_state.guide_active = true;
            m_guide_tool_state.anchor_line =
                Line(Point{width_f(), 0}, Point{width_f(), height_f()});
            m_guide_tool_state.guide_line = m_guide_tool_state.anchor_line;
            update();
        } else if (mouse_screen.x < RULER_WIDTH_PIXELS) {
            qDebug() << "On vertical rule (left)";
            m_guide_tool_state.guide_active = true;
            m_guide_tool_state.anchor_line = Line(Point{0, 0}, Point{0, height_f()});
            m_guide_tool_state.guide_line = m_guide_tool_state.anchor_line;
            update();
        } else if (mouse_screen.y > (height() - RULER_WIDTH_PIXELS)) {
            qDebug() << "On horizontal rule (bottom)";
            m_guide_tool_state.guide_active = true;
            m_guide_tool_state.anchor_line =
                Line(Point{0, height_f()}, Point{width_f(), height_f()});
            m_guide_tool_state.guide_line = m_guide_tool_state.anchor_line;
            update();
        } else if (mouse_screen.y < RULER_WIDTH_PIXELS) {
            qDebug() << "On horizontal rule (top)";
            m_guide_tool_state.guide_active = true;
            m_guide_tool_state.anchor_line = Line(Point{0, 0}, Point{width_f(), 0});
            m_guide_tool_state.guide_line = m_guide_tool_state.anchor_line;
            update();
        }
        break;
    }
    case Tool::rectangle: {
        qDebug() << "PRESS: RECT: " << x << ", " << y;

        if (!m_rect_tool_state.rect_active) {
            m_rect_tool_state.rect_active = true;
            m_rect_tool_state.p1 = mouse_world;
            m_rect_tool_state.p2 = mouse_world;
            update();
            setMouseTracking(true);
        } else {
            m_rect_tool_state.p2 = mouse_world;
            m_rect_tool_state.rect_active = false;
            m_model.rects.emplace_back(
                RectObj{Rect::from_two_points(m_rect_tool_state.p1, m_rect_tool_state.p2)});
            update();
        }

        break;
    }

    default:
        break;
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *event) {
    switch (m_selected_tool) {
    case Tool::draw_point: {
        break;
    }
    case Tool::hand: {
        // hand tool is for camera control.
        if (m_hand_tool_state == HandToolState::pressed) {
            m_hand_tool_state = HandToolState::idle;
        }
        break;
    }
    case Tool::move: {
        break;
    }
    case Tool::guide: {
        qDebug() << "GUIDE: RELEASE";
        if (std ::exchange(m_guide_tool_state.guide_active, false)) {
            m_model.guides.emplace_back(GuideObj{m_guide_tool_state.guide_line});
            update();
        }
    }
    case Tool::rectangle: {
        break;
    }
    default: {
        break;
    }
    }
}

void CanvasWidget::wheelEvent(QWheelEvent *event) {
    // Most mouse types work in increments of 15.0 degress but Qt returns eights of degree.
    // We assume that 15 degrees will correspond to 0.1 scale so minimal increment of wheel
    // results in + 0.1 or -0.1 zoom.

    const auto delta_degress = event->angleDelta().y() / 8;
    const auto delta_zoom = (delta_degress / 15.0) / 10.0; // mapped to -1/+1 for mouse mouses

    const auto x = event->position().x();
    const auto y = event->position().y();

    if (m_selected_tool == Tool::draw_point) {
        // ..
    } else if (m_selected_tool == Tool::hand) {
        if (m_hand_tool_state == HandToolState::idle) {
            if (!m_zoom_center_opt) {
                m_zoom_center_opt = {x, y};
            }

            m_scale += delta_zoom;
            qDebug() << "m_zoom: " << m_scale;
            update();
        }
    }
}

void CanvasWidget::render_background(QPainter *painter, QPaintEvent *event) {
    QBrush brush{QColor{235, 235, 235}};
    painter->fillRect(event->rect(), brush);
}

void CanvasWidget::render_handles(QPainter *painter, QPaintEvent *) {
    for (auto &p : m_model.points) {
        const double size = 10 / m_scale;
        const auto half_size = size / 2;

        QRectF point_rect{p.pt.x - half_size, p.pt.y - half_size, size, size};
        QBrush point_brush{QColor{255, 0, 0}};

        if (is_object_selected(p)) {
            point_brush.setColor(QColor(255, 255, 255));
        }
        // qDebug() << "point.rect: " << point_rect;
        painter->fillRect(point_rect, point_brush);

        // TODO: put in boolean (if m_debug_mode)
        QRectF hit_bbox = to_qrectf(select_bbox(p.pt, SELECT_TOOL_HIT_BBOX / m_scale));
        QPen hit_bbox_pen{QColor{100, 100, 100}};
        hit_bbox_pen.setWidth(0.5 / m_scale);
        painter->setPen(hit_bbox_pen);
        painter->drawRect(hit_bbox);
    }
}

void CanvasWidget::render_debug_elements(QPainter *painter, QPaintEvent *) {
    // Render bounding box for lines
    for (auto &line_obj : m_model.lines) {
        auto [a, b] = line_obj.l.endpoints();
        double size = 20.0 / m_scale;
    }

    for (auto p : m_projection_points) {
        draw_colored_point(painter, p, QColor(100, 100, 100));
    }

    if (!m_hitting_line_id.empty()) {
        auto it = std::find_if(m_model.lines.begin(), m_model.lines.end(),
                               [&](auto &l) { return l.id == m_hitting_line_id; });
        if (it != m_model.lines.end()) {
            draw_colored_line(painter, it->l.a, it->l.b, QColor(255, 0, 0));
        }
    }
}

void CanvasWidget::render_rulers(QPainter *painter, QPaintEvent *) {
    draw_colored_line(painter, Point{RULER_WIDTH_PIXELS, 0},
                      Point{RULER_WIDTH_PIXELS, (double)height()}, QColor(100, 100, 100));
    draw_colored_line(painter, Point{(double)width() - RULER_WIDTH_PIXELS, 0},
                      Point{(double)width() - RULER_WIDTH_PIXELS, (double)height()},
                      QColor(100, 100, 100));
    draw_colored_line(painter, Point{0, RULER_WIDTH_PIXELS},
                      Point{(double)width(), RULER_WIDTH_PIXELS}, QColor(100, 100, 100));
    draw_colored_line(painter, Point{0, (double)height() - RULER_WIDTH_PIXELS},
                      Point{(double)width(), (double)height() - RULER_WIDTH_PIXELS},
                      QColor(100, 100, 100));
}

void CanvasWidget::render_guides(QPainter *painter, QPaintEvent *) {
    // Render currently active guide
    if (m_selected_tool == Tool::guide && m_guide_tool_state.guide_active) {
        draw_dashed_line(painter, m_guide_tool_state.guide_line, Blue, thicker_line_width());
    }
    // Render already placed/finalized guides
    for (auto &guide : m_model.guides) {
        draw_dashed_line(painter, scale_line(guide.line, 1 / m_scale), Blue, thicker_line_width());
    }
}
void CanvasWidget::render_rects(QPainter *painter, QPaintEvent *) {
    auto draw_measurements_for_rect = [painter](Rect rect) {
        v2 top_left{rect.x, rect.y};
        v2 top_right{rect.x + rect.width, rect.y};
        v2 bottom_left{rect.x, rect.y + rect.height};
        v2 width_label_pos = (top_left + top_right) / 2.0;
        v2 height_label_pos = (top_left + bottom_left) / 2.0;
        width_label_pos.y -= 15.0;
        height_label_pos.x -= 15.0;

        auto width_label_frame = Rect::from_center_and_dimensions(width_label_pos, 100, 50);
        //        draw_rect(painter, width_label_frame, QColor(255, 0, 0));
        auto height_label_frame = Rect::from_center_and_dimensions(height_label_pos, 100, 50);
        //        draw_rect(painter, height_label_frame, QColor(0, 255, 0));

        auto width_dist = len(v2(top_left, top_right));
        auto height_dist = len(v2(top_left, bottom_left));

        // rect width label
        painter->save();
        painter->setPen(QColor(100, 100, 100));
        painter->drawText(to_qrectf(width_label_frame), Qt::AlignCenter | Qt::AlignVCenter,
                          format_distance_display_text(width_dist).c_str());
        painter->restore();

        // rect height label
        painter->save();
        painter->setPen(QColor(100, 100, 100));
        painter->translate(to_qpointf(height_label_frame.center()));
        painter->rotate(-90.0);
        painter->translate(-height_label_frame.center());
        painter->drawText(to_qrectf(height_label_frame), Qt::AlignCenter | Qt::AlignVCenter,
                          format_distance_display_text(height_dist).c_str());
        painter->restore();
        // ..
    };

    if (m_rect_tool_state.rect_active) {
        // Render rect currently being drawed
        auto rect = Rect::from_two_points(m_rect_tool_state.p1, m_rect_tool_state.p2);
        draw_rect(painter, rect, QColor(100, 100, 100));
        // Also, draw dimensions of our rect. For this we find positions first.
        qDebug() << "RENDER: RECT: " << rect.width << "x" << rect.height;
        draw_measurements_for_rect(rect);
    }

    for (auto &rectObj : m_model.rects) {
        auto &geometry = rectObj.rect;
        draw_rect(painter, geometry, QColor(0, 0, 0));

        const auto flags = rectObj.flags;
        if (flags & ObjFlags::top_rect_line_move_howered) {
            draw_colored_line(painter, geometry.top_line(), HowerColor, thicker_line_width());
        } else if (flags & ObjFlags::bottom_rect_line_move_howered) {
            draw_colored_line(painter, geometry.bottom_line(), HowerColor, thicker_line_width());
        } else if (flags & ObjFlags::left_rect_line_move_howered) {
            draw_colored_line(painter, geometry.left_line(), HowerColor, thicker_line_width());
        } else if (flags & ObjFlags::right_rect_line_move_howered) {
            draw_colored_line(painter, geometry.right_line(), HowerColor, thicker_line_width());
        }

        if (flags & (ObjFlags::top_rect_line_move | ObjFlags::bottom_rect_line_move |
                     ObjFlags::left_rect_line_move | ObjFlags::right_rect_line_move)) {
            draw_rect(painter, rectObj.shadow_rect, LightGrey);
            draw_measurements_for_rect(rectObj.shadow_rect);
        }
    }
}

void CanvasWidget::render_lines(QPainter *painter, QPaintEvent *) {
    for (auto &line_obj : m_model.lines) {
        auto &[a, b] = line_obj.l;
        draw_colored_line(painter, a, b, Qt::black, thin_line_width());

        // Draw shadows for each line as well in this loop. While formally it is a model,
        // but this looks convinient at this modent.
        // TODO: optimize this loop if there is not moved shadows right now in the
        // EditorState. Like (if editorFlags & hasMovingShadows)
        if (line_obj.flags &
            (ObjFlags::moving | ObjFlags::a_endpoint_move | ObjFlags::b_endpoint_move)) {
            auto &shadow = line_obj.shadow_l;
            draw_colored_line(painter, shadow, QColor(80, 80, 80), thin_line_width());
            draw_colored_line(painter, Line{a, shadow.a}, QColor(200, 200, 200), thin_line_width());
            draw_colored_line(painter, Line{b, shadow.b}, QColor(200, 200, 200), thin_line_width());

            // line connecting centers of line and its shadow
            Point c1 = a + v2{a, b} / 2;
            Point c2 = shadow.a + v2{shadow.a, shadow.b} / 2;
            double dist = len(v2(c1, c2));
            draw_dashed_line(painter, c1, c2, QColor(150, 150, 150), thin_line_width());

            v2 c1c2_center = (c1 + c2) / 2;
            Rect r{c1c2_center.x, c1c2_center.y, 100, 100};
            draw_colored_point(painter, c1c2_center, Qt::black);
            auto diff = r.upper_left_corner() - r.center();

            r.x += diff.x;
            r.y += diff.y; // TODO: we need method adjust and method for constructing rect
                           // around center of given size.

            // Move rect just above the line so that text is on top of the line instead of
            // directly on the line.
            r.y -= 20;

            painter->save();

            v2 v1{c1, c2};
            double theta = -std::atan2(v1.y, v1.x); // the angle between v1 and X axis.
            double theta_degrees = theta / M_PI * 180.0;

            qDebug() << "theta=" << theta;

            if (theta > M_PI_2 && theta < M_PI) {
                qDebug() << "zone2";
                theta_degrees += 180.0;
            } else if (theta > -M_PI && theta < -M_PI_2) {
                qDebug() << "zone3";
                theta_degrees += 180.0;
            }

            qDebug() << "theta_degrees=" << theta_degrees;

            auto qr = to_qrectf(r);

            painter->translate(qr.center());
            painter->rotate(-theta_degrees);
            painter->translate(-qr.center());
            painter->drawText(qr, Qt::AlignCenter | Qt::AlignVCenter,
                              format_distance_display_text(dist).c_str());
            painter->restore();
        } else if (line_obj.flags & ObjFlags::howered) {
            draw_colored_line(painter, a, b, HowerColor, thicker_line_width());
        } else if (line_obj.flags &
                   (ObjFlags::a_endpoint_move_howered | ObjFlags::b_endpoint_move_howered)) {

            if (line_obj.flags & ObjFlags::a_endpoint_move_howered) {
                // a endpoint however
                draw_colored_point(painter, line_obj.l.a, HowerColor, 10.0);
            } else {
                // b endpoint however
                draw_colored_point(painter, line_obj.l.b, HowerColor, 10.0);
            }
        }
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

Point CanvasWidget::world_to_screen(Point p) {
    return to_point(to_qpointf(p) * get_transformation_matrix());
}

Point CanvasWidget::screen_to_world(Point p) {
    return to_point(to_qpointf(p) * get_transformation_matrix().inverted());
}

Line CanvasWidget::world_to_screen(Line p) {
    return Line(world_to_screen(p.a), world_to_screen(p.b));
}
Line CanvasWidget::screen_to_world(Line p) {
    return Line(screen_to_world(p.a), screen_to_world(p.b));
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
