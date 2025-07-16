#include "mainwindow.h"
#include "ui_main.h"

MainWindow* MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),  // Используем QMainWindow
      ui(new Ui::MainWindow)
{
    m_instance = this;
    
    // Инициализация UI из сгенерированного файла
    ui->setupUi(this);  // Теперь типы совпадают
    
    // Дополнительная инициализация
    initializeUI();
}

MainWindow::~MainWindow()
{
    delete ui;
    m_instance = nullptr;
}

void MainWindow::initializeUI()
{
    // Здесь будет инициализация элементов и привязка сигналов
    // Пока оставляем пустым
}