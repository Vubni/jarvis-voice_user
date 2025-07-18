#ifndef TRAYSYSTEM_H
#define TRAYSYSTEM_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include "CustomNotification.h"

class TraySystem : public QObject
{
    Q_OBJECT

public:
    explicit TraySystem(QObject *parent = nullptr);
    void setup(QApplication* app);
    void show();

private:
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QApplication* m_app;

private slots:
    void handleTrayActivation(QSystemTrayIcon::ActivationReason reason);
    void handleSettings();
    void handleAbout();
    void handleExit();
};

#endif // TRAYSYSTEM_H