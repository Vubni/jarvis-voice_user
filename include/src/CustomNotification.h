// CustomNotification.h
#ifndef CUSTOMNOTIFICATION_H
#define CUSTOMNOTIFICATION_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include "nlohmann/json.hpp"

class CustomNotification : public QWidget
{
    Q_OBJECT
public:
    explicit CustomNotification(const QString& title, 
                               const QString& message, 
                               QWidget* parent = nullptr,
                               int displayTime = -1);

    void showNotification();

signals:
    void clicked(); // Добавленный сигнал для кликов

private:
    bool m_isClosing = false;
    void setupUI(const QString& title, const QString& message);
    void setupAnimation();
    void closeNotification();
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override; // Обработка кликов мышью

    QTimer* m_timer;
    QPropertyAnimation* m_animation;
};

QIcon createSvgIcon(const QString &path, int size = 64);
void showCustomNotification(const QString& title, const QString& message, int displayTime = -1);
void showCustomNotification(const char* title, const char* message, int displayTime = -1);
void showCustomNotification(const std::string& title, const std::string& message, int displayTime = -1);
void showCustomNotification(const char* title, const nlohmann::json& jsonMessage, int displayTime = -1);

#endif // CUSTOMNOTIFICATION_H