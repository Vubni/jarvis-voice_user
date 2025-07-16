#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>  // Изменяем с QWidget на QMainWindow
#include "ui_main.h"

class MainWindow : public QMainWindow  // Наследуемся от QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow* instance() { return m_instance; }

private:
    Ui::MainWindow *ui;
    static MainWindow* m_instance;

    void initializeUI();
};

#endif // MAINWINDOW_H