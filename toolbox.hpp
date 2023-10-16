#pragma once

#include <QFrame>
#include <vector>
#include "types.hpp"

class QPushButton;
class QToolButton;

class ToolBox : public QFrame {
    Q_OBJECT
  public:
    explicit ToolBox(QWidget *parent = nullptr);
    Tool get_selected_tool() const { return m_selected_tool; }

  signals:
    void selected_tool_changed(Tool selected_tool);

  private:
    QToolButton *m_hand_tool;
    QToolButton *m_draw_point_tool;
    QToolButton *m_draw_line_tool;    
    QToolButton *m_select_tool;    
    Tool m_selected_tool = Tool::hand;
    std::vector<QToolButton *> m_tools;
};
