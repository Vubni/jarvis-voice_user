#ifndef GRADIENT_WIDGET_H
#define GRADIENT_WIDGET_H

#include <QWidget>

class GradientWidget : public QWidget {
    Q_OBJECT

public:
    explicit GradientWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};

#endif // GRADIENT_WIDGET_H