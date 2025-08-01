#include "mainwindow.h"
#include "ui_main.h"
#include <QCloseEvent>
#include <QWindowStateChangeEvent> // Для работы с состояниями окна
#include <iostream>
#include <QDesktopServices>
#include <QObject>
#include <QDebug>
#include "speech_recognition.h"

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
    open_page("home");
    replaceCheckBox(this, "horizontalLayout_4", "switch_mute");
    replaceCheckBox(this, "horizontalLayout_3", "switch_animated");
    replaceCheckBox(this, "horizontalLayout_15", "switch_language_speech");
    replaceCheckBox(this, "horizontalLayout_12", "switch_cache");

    create_button_connect("button_microphone", &MainWindow::microphone_action);
    create_button_connect("button_telegram", &MainWindow::clicked_telegram);
    create_button_connect("button_site", &MainWindow::clicked_site);
    create_button_connect("button_home", &MainWindow::clicked_home);
    create_button_connect("button_scenarios", &MainWindow::clicked_scenarious);
    create_button_connect("button_plugins", &MainWindow::clicked_plugins);
    create_button_connect("button_profile", &MainWindow::clicked_profile);
    create_button_connect("button_settings", &MainWindow::clicked_settings);
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

void MainWindow::create_button_connect(const QString& childName, void (MainWindow::*slot)()) {
    QPushButton *button = findChild<QPushButton*>(childName);
    if (button) {
        connect(button, &QPushButton::clicked, 
                this, slot);
    } else {
        qWarning() << "button_microphone not found during initialization!";
    }
}

void MainWindow::clicked_settings() {
    open_page("settings");
}

void MainWindow::clicked_profile() {
    open_page("profile");
}

void MainWindow::clicked_home() {
    open_page("home");
}

void MainWindow::clicked_scenarious() {
    open_page("development");
}

void MainWindow::clicked_plugins() {
    open_page("development");
}

void MainWindow::clicked_telegram() {
    QDesktopServices::openUrl(QUrl("https://t.me/javris_pc"));
}

void MainWindow::clicked_site() {
    QDesktopServices::openUrl(QUrl("https://vubni.com"));
}

void MainWindow::microphone_action()
{
    microphone_status = !microphone_status;
    QPushButton *button_microphone = findChild<QPushButton*>("button_microphone");
    if (!microphone_status){
        button_microphone->setIcon(QIcon(":images/microphone-off-white.png"));
        recording_enabled = false;
    } else {
        button_microphone->setIcon(QIcon(":images/microphone-white.png"));
        recording_enabled = true;
    }
}


void MainWindow::open_page(const QString page_name){
    QStackedWidget *stackedWidget = findChild<QStackedWidget*>("stackedWidget");
    if (!stackedWidget) {
        qWarning() << "stackedWidget not found during initialization!";
    }
    QWidget *targetPage = stackedWidget->findChild<QWidget*>(page_name);
    if (targetPage) {
        stackedWidget->setCurrentWidget(targetPage);
    }
}

void append_text_console(const QString text){
    MainWindow* mainWindow = MainWindow::instance();
    if (QPlainTextEdit *console = mainWindow->findChild<QPlainTextEdit*>("console")) {
        console->appendPlainText(text);
        console->ensureCursorVisible();
    }
}

void append_user_text_console(const string text){
    append_text_console(QString("🔵Вы: ") + QString::fromStdString(text));
}

void append_jarvis_text_console(const string text){
    append_text_console(QString("🟢Джарвис: ") + QString::fromStdString(text));
}