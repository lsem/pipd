#include "toolbox.hpp"

#include <QDebug>
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyleOption>
#include <QToolButton>
#include <QVBoxLayout>

ToolBox::ToolBox(QWidget *parent)
    : QWidget(parent)

{
    auto layout = new QVBoxLayout();

    auto add_tool = [this, layout](auto tool_name, auto tool_id) {
        auto tool = new QToolButton(this);
        tool->setCheckable(true);
        tool->setChecked(false);
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
                    qDebug() << "emit signal";
                    emit selected_tool_changed(m_selected_tool);
                }
            }
        });
        return tool;
    };

    m_hand_tool = add_tool("H", Tool::hand);
    m_draw_point_tool = add_tool("P", Tool::draw_point);
    m_draw_line_tool = add_tool("L", Tool::draw_line);
    m_select_tool = add_tool("S", Tool::select);

    m_hand_tool->setChecked(true);

    layout->addStretch();
    setLayout(layout);

    this->layout()->setContentsMargins(10, 30, 10, 10);
    this->setFixedWidth(55);
    this->setFixedHeight(300);
}

void ToolBox::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, width(), height()), 10, 10);
    painter.fillPath(path, QColor(150, 150, 150));
}

void ToolBox::mouseMoveEvent(QMouseEvent *event) {
    for (auto &t : m_tools) {
        if (t->underMouse()) {
            event->ignore();
            return;
        }
    }

    if (event->buttons() & Qt::LeftButton) {
        move(mapToParent(event->pos() - m_offset));
    }
}

void ToolBox::mousePressEvent(QMouseEvent *event) {
    for (auto &t : m_tools) {
        if (t->underMouse()) {
            event->ignore();
            return;
        }
    }

    m_offset = event->pos();
}
void ToolBox::mouseReleaseEvent(QMouseEvent *event) {}
