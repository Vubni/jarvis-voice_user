#include "TraySystem.h"
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <iostream>
#include "logger.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QPainter>
#include <QIcon>
#include <QSvgRenderer>

TraySystem::TraySystem(QObject *parent) 
    : QObject(parent),
      m_trayIcon(new QSystemTrayIcon(this)),
      m_trayMenu(new QMenu()),
      m_app(nullptr)
{

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray not available!";
        return;
    }

    // Инициализация иконки
    QIcon icon = createSvgIcon(":/images/logo.svg");;
    if (icon.isNull()) {
        log_error("Error load icon tray!");
    } else {
        log_info("Succesful load icon tray.");
    }
    
    m_trayIcon->setIcon(icon);
    m_trayIcon->setToolTip("Tray Application");
    
    // Настройка обработчика активации
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &TraySystem::handleTrayActivation);
}

void TraySystem::setup(QApplication* app)
{
    m_app = app;
    
    // Настройка приложения
    m_app->setApplicationName("Tray Application");
    m_app->setApplicationDisplayName("Tray App");
    m_app->setWindowIcon(createSvgIcon(":/images/logo.svg"));
    
    // Создание пунктов меню
    QAction* settingsAction = new QAction(QIcon::fromTheme("preferences-system"), "Настройки", this);
    connect(settingsAction, &QAction::triggered, this, &TraySystem::handleSettings);
    m_trayMenu->addAction(settingsAction);
    
    QAction* aboutAction = new QAction(QIcon::fromTheme("help-about"), "О программе", this);
    connect(aboutAction, &QAction::triggered, this, &TraySystem::handleAbout);
    m_trayMenu->addAction(aboutAction);
    
    m_trayMenu->addSeparator();
    
    QAction* exitAction = new QAction(QIcon::fromTheme("application-exit"), "Выход", this);
    connect(exitAction, &QAction::triggered, this, &TraySystem::handleExit);
    m_trayMenu->addAction(exitAction);
    
    // Установка меню
    m_trayIcon->setContextMenu(m_trayMenu);
}

void TraySystem::show()
{
    m_trayIcon->show();
    showCustomNotification(
        "Джарвис Запускается!",
        "С помощью уведомлений я буду общраться с вами."
    );
}

void TraySystem::handleTrayActivation(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        QMessageBox::information(nullptr, "Активация", "Двойной клик по иконке!");
    }
}

void TraySystem::handleSettings()
{
    QMessageBox::information(nullptr, "Настройки", "Окно настроек приложения");
}

void TraySystem::handleAbout()
{
    QMessageBox::about(nullptr, "О программе", "Версия 1.0\nПример приложения с треем");
}

void TraySystem::handleExit()
{
    if (m_app) {
        m_app->quit();
    }
}