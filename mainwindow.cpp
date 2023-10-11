#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "canvas.hpp"

#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

// Represents a model of what we are editing/displaying.
struct Model {};

// Basically, our renderer does not own a model but draws it. Can have its own
// state, e.g. some cache or something.
class Renderer {
public:
  void render(QPaintEvent *event, QPainter *painter) {
    QBrush brush{QColor{255, 255, 255}};
    painter->fillRect(event->rect(), brush);
  }
};

MainWindow::MainWindow(QWidget *parent)
    // Qt::FramelessWindowHint | Qt::WindowSystemMenuHint)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_renderer(std::make_unique<Renderer>()),
      m_model(std::make_unique<Model>()) {
  ui->setupUi(this);
  this->setWindowTitle(QString::fromUtf8("pipd"));
}

MainWindow::~MainWindow() = default;

void MainWindow::paintEvent(QPaintEvent *event) /*override*/ {
  QPainter painter;
  painter.begin(this);
  m_renderer->render(event, &painter);
  painter.end();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
}
void MainWindow::mousePressEvent(QMouseEvent *event) {
    // so we clicked somewhere..
    qDebug() << "clicked at " << event->x() << ", " << event->y();    
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {}
void MainWindow::moveEvent(QMoveEvent *event) {}
void MainWindow::wheelEvent(QWheelEvent *event) {}
