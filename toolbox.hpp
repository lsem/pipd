#pragma once

#include "types.hpp"
#include <QFrame>
#include <vector>
#include <tuple>

class QPushButton;
class QToolButton;

class ToolBox : public QWidget {
    Q_OBJECT
  public:
    explicit ToolBox(QWidget *parent = nullptr);
    Tool get_selected_tool() const { return m_selected_tool; }

  signals:
    void selected_tool_changed(Tool selected_tool);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    QColor m_neutral_color1;
    QColor m_neutral_color2;    
    QToolButton *m_hand_tool;
    QToolButton *m_draw_point_tool;
    QToolButton *m_draw_line_tool;
    QToolButton *m_select_tool;
    QToolButton *m_move_tool;
    QToolButton *m_guide_tool;
    Tool m_selected_tool = Tool::hand;
    std::vector<QToolButton *> m_tools;
    QPoint m_offset;
};
