#include "app_launcher.h"
#include <QFile>
#include <QMessageBox>
#include <QUiLoader>

QWidget* createMainWindow(const QString &uiFilePath) {
    // Загрузка .ui файла
    QFile uiFile(uiFilePath);
    if (!uiFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Ошибка", 
            "Не удалось открыть UI файл:\n" + uiFilePath + "\nОшибка: " + uiFile.errorString());
        return nullptr;
    }

    // Инициализация загрузчика интерфейса
    QUiLoader uiLoader;
    QWidget* widget = uiLoader.load(&uiFile);
    uiFile.close();

    if (!widget) {
        QMessageBox::critical(nullptr, "Ошибка", 
            "Ошибка загрузки интерфейса из файла:\n" + uiFilePath);
        return nullptr;
    }

    return widget;
}