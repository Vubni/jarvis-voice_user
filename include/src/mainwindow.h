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

signals:
    void appendConsoleRequested(QString text);

private slots:
    void microphone_action();
    void clicked_site();
    void clicked_telegram();
    void clicked_home();
    void clicked_scenarios();
    void clicked_plugins();
    void clicked_profile();
    void clicked_settings();
    void animate_action(bool checked);
    void mute_action(bool checked);
    void speech_en_switch(bool checked);
    void cache_switch(bool checked);
    void updateJarvis(const QString &text);
    void appendConsoleText(QString text);

private:
    Ui::MainWindow *ui;
    static MainWindow* m_instance;
    bool microphone_status = true;

    void initializeUI();
    void switch_init(const QString& childName, const bool param);
    void create_button_connect(const QString& childName, void (MainWindow::*slot)());
    void create_switch_connect(const QString& childName, void (MainWindow::*slot)(bool));
};

void append_text_console(const QString text);
void append_user_text_console(const string text);
void append_jarvis_text_console(const string text);

#endif // MAINWINDOW_H