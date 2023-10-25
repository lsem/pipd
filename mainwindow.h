#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class CanvasWidget;
class ToolBox;
class LayersWindow;





class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  protected:
    void resizeEvent(QResizeEvent *event);

  private:
    std::unique_ptr<Ui::MainWindow> ui;
    CanvasWidget *m_canvas_widget{};
    ToolBox *m_toolbox{};
};
#endif // MAINWINDOW_H
