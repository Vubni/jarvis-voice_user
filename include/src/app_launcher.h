#ifndef APP_LAUNCHER_H
#define APP_LAUNCHER_H

#include <QString>
#include <QWidget>

QWidget* createMainWindow(const QString &uiFilePath = "./design/main.ui");

#endif