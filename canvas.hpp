#pragma once

#include <QWidget>

class Canvas : public QWidget {
  Q_OBJECT
public:
  explicit Canvas(QWidget *parent = nullptr);
  // QSize minimumSizeHint() const override { return QSize(400, 200); }
  // QSize sizeHint() const override { return QSize(4, 100); }

protected:
  void paintEvent(QPaintEvent *event) override;

private:
};
