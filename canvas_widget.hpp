#pragma once

#include "types.hpp"
#include <QWidget>
#include <memory>

class Renderer;
class Model;

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
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Model> m_model;

  public: // TODO: fixme!
    struct StateData;
    std::unique_ptr<StateData> m_state_data;
};
