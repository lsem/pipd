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
    : QMainWindow(parent), ui(new Ui::MainWindow), m_canvas_widget(new CanvasWidget{this}),
      m_toolbox(new ToolBox{this}) {

    ui->setupUi(this);
    this->setWindowTitle(QString::fromUtf8("pipd"));

    m_canvas_widget->select_tool(m_toolbox->get_selected_tool());

    connect(m_toolbox, &ToolBox::selected_tool_changed, m_canvas_widget,
            &CanvasWidget::select_tool);

    m_toolbox->move(width() - 100, 30);
    m_toolbox->resize(m_toolbox->width(), 300);
    m_toolbox->raise();

    auto *horizontal_layout = new QHBoxLayout(centralWidget());
    horizontal_layout->setContentsMargins(0, 0, 0, 0);
    horizontal_layout->addWidget(m_canvas_widget);
}

MainWindow::~MainWindow() = default;
