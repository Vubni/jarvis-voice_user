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
        // Установка полноэкранного режима через setGeometry
        QRect screenGeometry = QApplication::primaryScreen()->geometry();
        setGeometry(screenGeometry);

        // Атрибуты прозрачности
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);

        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        
        // Отладка: вывод текущей геометрии
        qDebug() << "Window geometry:" << geometry();

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
        timer->start(30); // ~33 FPS
    }

protected:
    void paintEvent(QPaintEvent *) override {
        qDebug() << "paintEvent started";
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
        // Проверяем, существует ли QApplication (предполагаем, что он уже создан)
        if (QApplication::instance() == nullptr) {
            int argc = 1;
            char* argv[] = {(char*)"GradientWidget"};
            static QApplication app(argc, argv); // Инициализация QApplication
            // Сохраняем указатель на QApplication в статической переменной, если необходимо
            static bool appInitialized = true;
            // Не вызываем app.exec() здесь, так как он должен быть в main()
        }

        // Создаем и показываем виджет
        if (!animationWidget) {
            animationWidget = new GradientWidget();
            animationWidget->show();
            animationWidget->update(); // Принудительное обновление
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