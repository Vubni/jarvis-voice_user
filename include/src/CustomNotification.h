#ifndef CUSTOMNOTIFICATION_H
#define CUSTOMNOTIFICATION_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class CustomNotification : public QWidget
{
    Q_OBJECT
public:
    explicit CustomNotification(const QString& title, 
                               const QString& message, 
                               QWidget* parent = nullptr,
                               int displayTime = -1);

    void showNotification();

private:
    void setupUI(const QString& title, const QString& message);
    void setupAnimation();
    void closeNotification();
    void paintEvent(QPaintEvent* event) override;

    QTimer* m_timer;
    QPropertyAnimation* m_animation;
};

QIcon createSvgIcon(const QString &path, int size = 64);

#endif // CUSTOMNOTIFICATION_H