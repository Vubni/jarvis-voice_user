#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "ui_main.h"
#include "switch.h"
#include <iostream>

using namespace std;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void open_page(const QString page_name);
    static MainWindow* instance() { return m_instance; }
public slots:
    void showWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void microphone_action();
    void clicked_site();
    void clicked_telegram();
    void clicked_home();
    void clicked_scenarious();
    void clicked_plugins();
    void clicked_profile();
    void clicked_settings();

private:
    Ui::MainWindow *ui;
    static MainWindow* m_instance;
    bool microphone_status = true;

    void initializeUI();
    void create_button_connect(const QString& childName, void (MainWindow::*slot)());
};

void append_user_text_console(const string text);
void append_jarvis_text_console(const string text);

#endif // MAINWINDOW_H