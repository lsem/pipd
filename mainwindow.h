#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class Renderer;
class Model;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void moveEvent(QMoveEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  std::unique_ptr<Ui::MainWindow> ui;
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<Model> m_model;
};
#endif // MAINWINDOW_H
