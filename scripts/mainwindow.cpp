#include "mainwindow.h"
#include "ui_main.h"
#include <QCloseEvent>
#include <QWindowStateChangeEvent> // Для работы с состояниями окна
#include <iostream>
#include <QDesktopServices>
#include <QObject>
#include <QDebug>
#include "speech_recognition.h"
#include "settings.h"
#include "switch.h"
#include "authorization.h"
#include "installed_programs.h"
#include "logger.h"
#include "api.h"

MainWindow* MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    m_instance = this;
    ui->setupUi(this);
    initializeUI();
    setAttribute(Qt::WA_DeleteOnClose, false);

    connect(this, &MainWindow::appendConsoleRequested, 
            this, &MainWindow::appendConsoleText, 
            Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
    m_instance = nullptr;
}

void MainWindow::initializeUI()
{
    bool status_auth = checker_authorization();
    if (!status_auth) {
        open_authorization();
    } else {
        open_page("home");
    }
    QPushButton *button = findChild<QPushButton*>("button_home");
    button->setEnabled(false);
    lastClickedButton = button;

    QLabel *label_error_auth = findChild<QLabel*>("label_error_auth");
    label_error_auth->setHidden(true);

    replaceCheckBox(this, "horizontalLayout_4", "switch_mute");
    replaceCheckBox(this, "horizontalLayout_3", "switch_animated");
    replaceCheckBox(this, "horizontalLayout_15", "switch_speech_en");
    replaceCheckBox(this, "horizontalLayout_12", "switch_cache");

    create_button_connect("button_microphone", &MainWindow::microphone_action);
    create_button_connect("button_telegram", &MainWindow::clicked_telegram);
    create_button_connect("button_site", &MainWindow::clicked_site);
    create_button_connect("button_home", &MainWindow::clicked_home);
    create_button_connect("button_scenarios", &MainWindow::clicked_scenarios);
    create_button_connect("button_plugins", &MainWindow::clicked_plugins);
    create_button_connect("button_profile", &MainWindow::clicked_profile);
    create_button_connect("button_settings", &MainWindow::clicked_settings);

    create_button_connect("button_auth", &MainWindow::clicked_authorization);

    create_switch_connect("switch_animated", &MainWindow::animate_action);
    create_switch_connect("switch_mute", &MainWindow::mute_action);
    create_switch_connect("switch_speech_en", &MainWindow::speech_en_switch);
    create_switch_connect("switch_cache", &MainWindow::cache_switch);

    json settings = get_settings();
    switch_init("switch_animated", settings["animation"]);
    switch_init("switch_mute", settings["mute"]);
    switch_init("switch_speech_en", settings["speech_en"]);
    switch_init("switch_cache", settings["save_cache"]);

    QPlainTextEdit *console = findChild<QPlainTextEdit*>("console");
    if (console) {
        QPalette pal = console->palette();
        pal.setColor(QPalette::PlaceholderText, QColor("#B0B0B0"));
        console->setPalette(pal);
    } else {
        qWarning() << "Switch not found during initialization!";
    }

    QLineEdit *lineEdit = findChild<QLineEdit*>("lineEdit_jarvis");
    if (lineEdit) {
        connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::updateJarvis);
    } else {
        qWarning() << "Switch not found during initialization!";
    }
    lineEdit->setText(QString::fromStdString(settings["prefix"]));
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

void MainWindow::switch_init(const QString& childName, const bool param) {
    Switch *cb = findChild<Switch*>(childName);
    if (cb) {
        cb->setChecked(param);
    } else {
        qWarning() << "Switch not found during initialization!";
    }
}

void MainWindow::create_switch_connect(const QString& childName, void (MainWindow::*slot)(bool)) {
    Switch *cb = findChild<Switch*>(childName);
    if (cb) {
        connect(cb, &Switch::clicked,
                this, slot);
    } else {
        qWarning() << "Switch not found during initialization!";
    }
}

void MainWindow::create_button_connect(const QString& childName, void (MainWindow::*slot)()) {
    QPushButton *button = findChild<QPushButton*>(childName);
    if (button) {
        connect(button, &QPushButton::clicked, 
                this, slot);
    } else {
        qWarning() << "button not found during initialization!";
    }
}

void MainWindow::clicked_settings() {
    open_page("settings");
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (lastClickedButton && lastClickedButton != clickedButton) {
        // Разблокировать предыдущую кнопку
        lastClickedButton->setChecked(false);
        lastClickedButton->setEnabled(true);
    }
    clickedButton->setEnabled(false);
    lastClickedButton = clickedButton;
}

void MainWindow::clicked_profile() {
    open_page("profile");
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (lastClickedButton && lastClickedButton != clickedButton) {
        // Разблокировать предыдущую кнопку
        lastClickedButton->setChecked(false);
        lastClickedButton->setEnabled(true);
    }
    clickedButton->setEnabled(false);
    lastClickedButton = clickedButton;
}

void MainWindow::clicked_home() {
    open_page("home");
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (lastClickedButton && lastClickedButton != clickedButton) {
        // Разблокировать предыдущую кнопку
        lastClickedButton->setChecked(false);
        lastClickedButton->setEnabled(true);
    }
    clickedButton->setEnabled(false);
    lastClickedButton = clickedButton;
}

void MainWindow::clicked_scenarios() {
    open_page("scenarios");
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (lastClickedButton && lastClickedButton != clickedButton) {
        lastClickedButton->setChecked(false);
        lastClickedButton->setEnabled(true);
    }
    clickedButton->setEnabled(false);
    lastClickedButton = clickedButton;
}

void MainWindow::clicked_plugins() {
    open_page("plugins");
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (lastClickedButton && lastClickedButton != clickedButton) {
        lastClickedButton->setChecked(false);
        lastClickedButton->setEnabled(true);
    }
    clickedButton->setEnabled(false);
    lastClickedButton = clickedButton;
}



void MainWindow::open_authorization() {
    open_page("auth");
    QFrame *frame = findChild<QFrame*>("frame");
    frame->setEnabled(false);
}


void MainWindow::clicked_authorization() {
    QLineEdit *login_line = findChild<QLineEdit*>("lineEdit_auth_login");
    string login = login_line->text().toStdString();
    if (login.empty()) {
        QLabel *label_error_auth = findChild<QLabel*>("label_error_auth");
        label_error_auth->setText("Введите логин.");
        label_error_auth->setHidden(false);
        return;
    }
    QLineEdit *password_line = findChild<QLineEdit*>("lineEdit_auth_password");
    string password = password_line->text().toStdString();
    if (password.empty()) {
        QLabel *label_error_auth = findChild<QLabel*>("label_error_auth");
        label_error_auth->setText("Введите пароль.");
        label_error_auth->setHidden(false);
        return;
    }
    bool status = authorization(login, password);
    if (status) {
        QFrame *frame = findChild<QFrame*>("frame");
        frame->setEnabled(true);
        open_page("home");
        status_auth = true;
        nlohmann::json pathsPrograms = InstalledPrograms::GetInstalledPrograms();
        bool result = create_session(pathsPrograms);
        if (result){
            log_info("Successful Create session.");
        } else {
            log_error("Failed to create session.");
        }
    } else {
        QLabel *label_error_auth = findChild<QLabel*>("label_error_auth");
        label_error_auth->setText("Неверный логин или пароль.");
        label_error_auth->setHidden(false);
    }
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

void MainWindow::updateJarvis(const QString &text){
    json setting = {{"prefix", text.toStdString()}};
    update_settings(setting);
    save_settings();
}

void MainWindow::animate_action(bool checked){
    json setting = {{"animation", checked}};
    update_settings(setting);
    save_settings();
}

void MainWindow::mute_action(bool checked){
    json setting = {{"mute", checked}};
    update_settings(setting);
    save_settings();
}

void MainWindow::speech_en_switch(bool checked){
    json setting = {{"speech_en", checked}};
    update_settings(setting);
    save_settings();
}

void MainWindow::cache_switch(bool checked){
    json setting = {{"save_cache", checked}};
    update_settings(setting);
    save_settings();
}

void MainWindow::appendConsoleText(QString text) {
    if (QPlainTextEdit *console = findChild<QPlainTextEdit*>("console")) {
        console->appendPlainText(text);
        console->ensureCursorVisible();
    }
}

void append_text_console(const QString text) {
    MainWindow* mainWindow = MainWindow::instance();
    if (mainWindow) {
        emit mainWindow->appendConsoleRequested(text);
    }
}

void append_user_text_console(const string text){
    append_text_console(QString("🔵Вы: ") + QString::fromStdString(text));
}

void append_jarvis_text_console(const string text){
    append_text_console(QString("🟢Джарвис: ") + QString::fromStdString(text));
}