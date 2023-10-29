#pragma once

#include "MoveTool.hpp"
#include "types.hpp"
#include <QWidget>
#include <memory>
#include <optional>

enum class CanvasState { idle, drawing };

enum class HandToolState { idle, pressed, zooming };
enum class DrawLineState { waiting_point_a, point_a_placed };

// How we are supposed to implement a function of group select and move:
//  suppose we selected two lines and three points, and now we want to support some group operation.
// Say, we want to group it.

// Represents an editor editor. Whenver we need to change the model, we do it through a command.
struct Command {
  public:
    struct Base {
        virtual ~Base() = default;
        virtual std::unique_ptr<Base> clone() = 0;
        virtual void execute() = 0;
        virtual void undo() = 0;
    };
    template <class T> struct Derived : public Base {
        Derived(T o) : m_o(std::move(o)) {}
        virtual std::unique_ptr<Base> clone() override { return std::make_unique<Derived<T>>(m_o); }
        virtual void execute() { m_o.execute(); }
        virtual void undo() { m_o.undo(); }
        T m_o;
    };

  public:
    template <class T> Command(T t) : m_impl(std::make_unique<Derived<T>>(std::move(t))) {}
    Command(const Command &) : m_impl(m_impl->clone()) {}
    void execute() { return m_impl->execute(); }
    void undo() { return m_impl->undo(); }

  private:
    std::unique_ptr<Base> m_impl;
};

class MoveLineCommand {
  public:
    void execute() {}
    void undo() {}
};

inline void test() {

    MoveLineCommand move_cmd;

    std::vector<Command> cmds;
    cmds.push_back(std::move(move_cmd));
}

class CanvasWidget : public QWidget, public IToolHost {
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

  public: // IToolHost
    virtual void ToolHost__update() override { update(); }
    virtual void ToolHost__enable_mouse_tracking(bool v) override { setMouseTracking(v); }

  private:
    void render_background(QPainter *painter, QPaintEvent *);
    void render_handles(QPainter *painter, QPaintEvent *);
    void render_lines(QPainter *painter, QPaintEvent *);
    void render_debug_elements(QPainter *painter, QPaintEvent *);
    void render_rulers(QPainter *painter, QPaintEvent *);
    void render_guides(QPainter *painter, QPaintEvent *);

    double scaled(double x) const { return x / m_scale; }
    double thin_line_width() const { return scaled(1.0); }
    double thicker_line_width() const { return scaled(2.0); }

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

    double width_f() const { return static_cast<double>(width()); }
    double height_f() const { return static_cast<double>(height()); }

  private:
    CanvasState m_state = CanvasState::idle;
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
    std::vector<Point> m_projection_points;

    std::vector<std::string> m_selected_objects;
    std::string m_hitting_line_id;

    Model m_model;
    MoveTool m_move_tool;

    struct {
        bool guide_active = false; // whether guide is current being displayed
        Line anchor_line;          // the line from which a guide originated
        Line guide_line;           // current position of a guide
    } m_guide_tool_state;
};
