#include "layers_window.hpp"

LayersWindow::LayersWindow(QWidget *parent) : QListWidget(parent) {
    setMaximumWidth(150);
    addItem("one");
    addItem("two");
    addItem("three");
}
