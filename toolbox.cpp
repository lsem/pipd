#include "toolbox.hpp"

#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

ToolBox::ToolBox(QWidget *parent) : QFrame(parent) {
    setMaximumWidth(50);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    auto layout = new QVBoxLayout();

    auto add_tool = [this, layout](auto tool_name, auto tool_id) {
        auto tool = new QToolButton(this);
        tool->setCheckable(true);
        tool->setText(tool_name);
        m_tools.emplace_back(tool);
        layout->addWidget(tool);
        connect(tool, &QToolButton::toggled, this, [tool, tool_id, this](bool checked) {
            if (checked) {
                tool->setChecked(true);

                for (auto t : m_tools) {
                    if (t != tool && t->isChecked()) {
                        t->setChecked(false);
                    }
                }
                if (m_selected_tool != tool_id) {
                    m_selected_tool = tool_id;
                    emit selected_tool_changed(m_selected_tool);
                }
            }
        });
        return tool;
    };

    m_hand_tool = add_tool("H", Tool::hand);
    m_draw_tool = add_tool("D", Tool::draw);

    m_hand_tool->setChecked(true);

    layout->addStretch();

    setLayout(layout);
}
