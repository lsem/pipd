#pragma once

#include "types.hpp"
#include <QListWidget>
#include <vector>

class LayersWindow : public QListWidget {
    Q_OBJECT
  public:
    explicit LayersWindow(QWidget *parent = nullptr);

  private:
};
