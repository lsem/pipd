#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "canvas_widget.hpp"
#include "layers_window.hpp"
#include "toolbox.hpp"

#include <QDebug>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    // Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_canvas_widget(new CanvasWidget{this}),
      m_toolbox(new ToolBox{this}), m_layers_window{new LayersWindow{this}} {
    ui->setupUi(this);
    this->setWindowTitle(QString::fromUtf8("pipd"));

    m_canvas_widget->select_tool(m_toolbox->get_selected_tool());

    connect(m_toolbox, &ToolBox::selected_tool_changed, m_canvas_widget,
            &CanvasWidget::select_tool);

    auto control_and_tools_layout = new QVBoxLayout();
    control_and_tools_layout->addWidget(m_toolbox);
    control_and_tools_layout->addWidget(m_layers_window);

    auto *horizontal_layout = new QHBoxLayout(centralWidget());
    horizontal_layout->addWidget(m_canvas_widget);
    horizontal_layout->addLayout(control_and_tools_layout);
}

MainWindow::~MainWindow() = default;
