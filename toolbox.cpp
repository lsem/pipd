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

const auto selected_tool_color = QColor(141, 153, 147);

void set_button_color(QToolButton &btn, QColor c1, QColor c2) {
    auto p = btn.palette();
    p.setBrush(QPalette::Base, c1);
    p.setBrush(QPalette::Button, c2);
    btn.setPalette(p);
}

std::tuple<QColor, QColor> get_button_color(QToolButton &btn) {
    auto p = btn.palette();
    auto c1 = p.brush(QPalette::Base);
    auto c2 = p.brush(QPalette::Button);
    return {c1.color(), c2.color()};
}

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
                        //                  set_button_color(*t, m_neutral_color1,
                        //                  m_neutral_color2);
                    }
                }
                if (m_selected_tool != tool_id) {
                    m_selected_tool = tool_id;
                    //                    set_button_color(*tool, selected_tool_color,
                    //                    selected_tool_color);
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
    m_move_tool = add_tool("M", Tool::move);

    auto [c1, c2] = get_button_color(*m_hand_tool);
    m_neutral_color1 = c1;
    m_neutral_color2 = c2;

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
    painter.fillPath(path, QColor(200, 200, 200));
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

void ToolBox::mousePressEvent(QMouseEvent *event) { m_offset = event->pos(); }
void ToolBox::mouseReleaseEvent(QMouseEvent *event) {}
