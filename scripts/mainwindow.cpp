#include "mainwindow.h"
#include "ui_main.h"
#include <QCloseEvent>
#include <QWindowStateChangeEvent> // Для работы с состояниями окна
#include <iostream>

MainWindow* MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    m_instance = this;
    ui->setupUi(this);
    initializeUI();
    setAttribute(Qt::WA_DeleteOnClose, false);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_instance = nullptr;
}

void MainWindow::initializeUI()
{
    replaceCheckBox(this, "horizontalLayout_4", "switch_mute");
    replaceCheckBox(this, "horizontalLayout_3", "switch_animated");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void MainWindow::showWindow()
{
    if (!m_instance) return;

    // Гарантируем, что окно будет видимым и активным
    if (isMinimized() || (windowState() & Qt::WindowMinimized)) {
        showNormal();  // Восстанавливаем из свернутого состояния
    } else if (isHidden()) {
        show();        // Показываем если было скрыто
    }
    
    // Принудительно активируем окно
    activateWindow();   // Запрашиваем фокус ввода
    raise();            // Поднимаем поверх других окон
    
    // Дополнительные меры для гарантии активации
    QApplication::processEvents(); // Обрабатываем события
    setFocus();                    // Устанавливаем фокус
}