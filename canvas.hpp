#pragma once

#include <QWidget>

class Canvas : public QWidget {
public:
  explicit Canvas(QWidget *parent = nullptr);
  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

public slots:

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  // ..
};
