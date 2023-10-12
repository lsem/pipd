#pragma once

#include <QFrame>

class QPushButton;

class ToolBox : public QFrame {
    Q_OBJECT
  public:
    explicit ToolBox(QWidget *parent = nullptr);

  private:
    QPushButton *m_one;
    QPushButton *m_two;
};
