#pragma once

#include "types.hpp"
#include <QWidget>
#include <memory>
#include <optional>

enum class CanvasState { idle, drawing };

enum class HandToolState { idle, pressed, zooming };
enum class DrawLineState { waiting_point_a, point_a_placed };

class CanvasWidget : public QWidget {
    Q_OBJECT

  public:
    CanvasWidget(QWidget *parent = nullptr);
    ~CanvasWidget();

  public slots:
    void select_tool(Tool tool);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

  private:
    void render_background(QPainter *painter, QPaintEvent *);
    void render_handles(QPainter *painter, QPaintEvent *);
    void render_lines(QPainter *painter, QPaintEvent *);

    Point world_to_screen(Point p);
    Point screen_to_world(Point p);
    Line world_to_screen(Line p);
    Line screen_to_world(Line p);

    void mark_object_selected(const PointObj &o);
    void mark_object_selected(const LineObj &o);
    void unmark_object_selected(const PointObj &o);
    void unmark_object_selected(const LineObj &o);
    void select_object_by_id_impl(const std::string &id);
    void deselect_object_by_id_impl(const std::string &id);

    bool is_object_selected(const PointObj &o);
    bool is_object_selected(const LineObj &o);

    QTransform get_transformation_matrix() const;

  private:
    CanvasState m_state = CanvasState::idle;
    // std::vector<Point> m_current_shape;
    Tool m_selected_tool = Tool::hand;

    int m_translate_x = 0;
    int m_translate_y = 0;
    double m_scale = 1.0;

    std::optional<Point> m_zoom_center_opt;

    HandToolState m_hand_tool_state = HandToolState::idle;
    int m_prev_x = 0;
    int m_prev_y = 0;

    DrawLineState m_draw_line_state;
    Point m_line_point_a{0, 0};
    Point m_line_point_b{0, 0};

    std::vector<PointObj> m_points;
    std::vector<LineObj> m_lines;

    std::vector<std::string> m_selected_objects;
};
