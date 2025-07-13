#include "gradientwidget.h"
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QLinearGradient>
#include <QTime>
#include <QPainterPath>
#include <cmath>
#include <QDebug>
#include <QScreen>

class GradientWidget : public QWidget {
    Q_OBJECT

public:
    explicit GradientWidget(QWidget *parent = nullptr) : QWidget(parent) {
        // Установка полноэкранного режима
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        setGeometry(screenGeometry);

        // Критические атрибуты окна
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_ShowWithoutActivating); // Предотвращает активацию окна
        setAttribute(Qt::WA_X11DoNotAcceptFocus);   // Для X11
        setAttribute(Qt::WA_MacAlwaysShowToolWindow); // Для macOS совместимость

        // Комбинированные флаги окна
        setWindowFlags(
            Qt::FramelessWindowHint |
            Qt::WindowStaysOnTopHint |
            Qt::ToolTip |               // Скрывает из панели задач и Alt+Tab
            Qt::WindowDoesNotAcceptFocus // Предотвращает фокусировку
        );

        // Отладка геометрии
        qDebug() << "Window geometry:" << geometry();

        // Настройка анимации
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() { this->update(); });
        timer->start(30);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        // Проверка на пустую область
        if (rect().isEmpty())
            return;

        QPainter painter(this);

        const int borderWidth = 1;

        qint64 elapsed = QTime::currentTime().msecsSinceStartOfDay();
        qreal time = elapsed / 1000.0;

        qreal angularSpeed = 2 * M_PI / 10;
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
};

// Статический указатель на экземпляр виджета
namespace {
    GradientWidget* animationWidget = nullptr;
}

extern "C" {
    void OnAnimation() {
        // Проверяем, существует ли QApplication
        if (!QApplication::instance()) {
            qWarning() << "QApplication must be initialized before calling OnAnimation.";
            return;
        }

        // Создаем и показываем виджет
        if (!animationWidget) {
            animationWidget = new GradientWidget();
            animationWidget->show();
            animationWidget->update();
        }
    }

    void OffAnimation() {
        if (animationWidget) {
            animationWidget->hide();
            animationWidget->deleteLater();
            animationWidget = nullptr;
            QApplication::processEvents(); // Обработка отложенных событий
        }
    }
}

#include "gradientwidget.moc"