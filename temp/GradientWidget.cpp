#define _USE_MATH_DEFINES
#include "GradientWidget.h"
#include <QPainter>
#include <QTimer>
#include <QLinearGradient>
#include <QTime>
#include <QPainterPath>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <cmath> // M_PI будет доступен после _USE_MATH_DEFINES

GradientWidget::GradientWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    setGeometry(screenGeometry);

    qDebug() << "Window geometry:" << geometry();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timer->start(30); // ~33 FPS
}

void GradientWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    const int borderWidth = 1;

    qint64 elapsed = QTime::currentTime().msecsSinceStartOfDay();
    qreal time = elapsed / 1000.0;

    qreal angularSpeed = 2 * M_PI / 10; // Используем M_PI вместо qPi()
    qreal theta = angularSpeed * time;

    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    qreal R = qSqrt(cx * cx + cy * cy) + 100;

    qreal x1 = cx + R * cos(theta);
    qreal y1 = cy + R * sin(theta);
    qreal x2 = cx - R * cos(theta);
    qreal y2 = cy - R * sin(theta);

    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0.0, Qt::red);
    gradient.setColorAt(0.2, Qt::yellow);
    gradient.setColorAt(0.4, Qt::green);
    gradient.setColorAt(0.6, Qt::cyan);
    gradient.setColorAt(0.8, Qt::blue);
    gradient.setColorAt(1.0, Qt::magenta);

    QPainterPath outer;
    outer.addRect(rect());

    QRect innerRect = rect().adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth);
    QPainterPath inner;
    inner.addRect(innerRect);

    QPainterPath framePath = outer.subtracted(inner);
    painter.fillPath(framePath, gradient);
}