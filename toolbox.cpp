#include "toolbox.hpp"

#include <QPushButton>
#include <QVBoxLayout>

ToolBox::ToolBox(QWidget *parent) : QFrame(parent) {
    setMaximumWidth(200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    m_one = new QPushButton("One");
    m_two = new QPushButton("Two");

    auto layout = new QVBoxLayout();
    layout->addWidget(m_one);
    layout->addWidget(m_two);

    setLayout(layout);
}
