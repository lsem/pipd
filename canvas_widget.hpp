#pragma once

#include "types.hpp"
#include <QWidget>
#include <memory>

enum class CanvasState { idle, drawing };

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
    void moveEvent(QMoveEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

  private:
    void render_background(QPainter *painter, QPaintEvent *);
    void render_handles(QPainter *painter, QPaintEvent *);
    void render_lines(QPainter *painter, QPaintEvent *);

  private:
    CanvasState m_state = CanvasState::idle;
    std::vector<Point> m_current_shape;
};
