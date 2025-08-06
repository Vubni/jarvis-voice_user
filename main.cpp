#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <windows.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <string>
#include <sstream>
#include <cctype>
#include <filesystem>
#include <iomanip>

#include "logger.h"
#include "http_client.h"
#include "api.h"
#include "core.h"
#include "speech_recognition.h"

#include <QApplication>
#include <QThread>
#include <QResource>
#include "gradientwidget.h"
#include "animationcontroller.h"
#include "WindowLister.h"
#include "app_launcher.h"
#include "mainwindow.h"
#include "TraySystem.h"
#include <QtSvg>
#include "autostart.h"
#include <QStyleFactory>
#include "settings.h"

#include <dwmapi.h> // Windows API
#pragma comment(lib, "dwmapi.lib")

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

AnimationController controller;
MainWindow* globalMainWindow = nullptr;

VoskModel* model = nullptr;
VoskModel* model_en = nullptr;
pv_recorder_t* recorder = nullptr;
std::atomic<bool> recognition_active(false);

void setTitleBarColor(HWND hwnd, const QColor &color) {
    const DWORD dwAttribute = DWMWA_CAPTION_COLOR; // Атрибут цвета заголовка
    const COLORREF cref = RGB(color.red(), color.green(), color.blue());
    DwmSetWindowAttribute(hwnd, dwAttribute, &cref, sizeof(cref));
}

int initialize(){
    mainCommands();

    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    AutoStart::AddToStartup(L"Jarvis Vubni", exePath);

    log_info("Create session..");
    nlohmann::json pathsPrograms = InstalledPrograms::GetInstalledPrograms();
    bool result = create_session(pathsPrograms);
    if (result){
        log_info("Successful Create session.");
    } else {
        log_error("Failed to create session. Program will exit.");
    }

    vector<HWND> temp = GetActiveWindows();
    char title[256];
    for (HWND hwnd : temp) {
        GetWindowTextA(hwnd, title, sizeof(title));
        printf("HWND: 0x%p, Заголовок: %s\n", hwnd, title);
    }

    {
        log_info("Create RU model..");
        json settings = get_settings();
        model = vosk_model_new(("./models/" + settings["model"].get<std::string>()).c_str());
        if (!model) {
            cerr << "Failed to load Vosk model" << endl;
            return -1;
        }

        recognizer_ru = vosk_recognizer_new(model, 16000.0f);
        vosk_recognizer_set_max_alternatives(recognizer_ru, 0);
    }

    {
        log_info("Create EN model..");
        model_en = vosk_model_new("./models/small_en");
        if (!model_en) {
            cerr << "Failed to load Vosk model" << endl;
            return -1;
        }

        recognizer_en = vosk_recognizer_new(model_en, 16000.0f);
        vosk_recognizer_set_max_alternatives(recognizer_en, 0);
    }

    if (pv_recorder_init(512, -1, 5, &recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "PvRecorder init error" << endl;
        vosk_model_free(model);
        vosk_model_free(model_en);
        return -1;
    }
    if (pv_recorder_start(recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "Recording start error" << endl;
        pv_recorder_delete(recorder);
        vosk_model_free(model);
        vosk_model_free(model_en);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    load_settings("settings.json");

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, QColor("#000000"));
    
    qApp->setPalette(darkPalette);

    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    Q_INIT_RESOURCE(resources);
    // QResource::registerResource("resources.rcc");

    log_info("Starting Jarvis Voice application...");
     
    // Создаем главное окно через фабричную функцию
    MainWindow* mainWindow = static_cast<MainWindow*>(createMainWindow());
    if (!mainWindow) {
        return -1;
    }
    if (HWND hwnd = reinterpret_cast<HWND>(mainWindow->winId())) {
        setTitleBarColor(hwnd, QColor(25,25,25));
    }
    mainWindow->show();
    globalMainWindow = mainWindow;

    log_info("Create controller to animation...");
    QObject::connect(&controller, &AnimationController::toggleAnimation, 
                    [&](bool isActive) {
        if (isActive) {
            OnAnimation();
        } else {
            OffAnimation();
        }
    });

    log_info("Create tray system...");
    TraySystem tray;
    tray.setup(&app);
    tray.show();


    log_info("Initialize Jarvis...");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (!filesystem::exists("logs")) filesystem::create_directory("logs");

    error_log_file.open("logs/error.log", ios_base::app);
    if (error_log_file.is_open()) {
        log_info("Application started");
    }

    recognition_active = true;
    thread thread_recognition = std::thread([]() {
        initialize();
        
        if (!recognition_active) return;
        
        cout << "Recording started... Speak into the microphone." << endl;
        start_speech_recognition(recorder);
    });
    int qt_result = app.exec();

    // Остановка распознавания перед выходом
    recognition_active = false;
    stop_speech_recognition();
    
    if (thread_recognition.joinable()) {
        thread_recognition.join();
    }
    // Остановка распознавания и очистка ресурсов
    stop_speech_recognition();
    pv_recorder_stop(recorder);
    pv_recorder_delete(recorder);
    vosk_recognizer_free(recognizer_ru);
    vosk_recognizer_free(recognizer_en);
    vosk_model_free(model);
    vosk_model_free(model_en);

    if (error_log_file.is_open()) {
        log_error("Application terminated");
        error_log_file.close();
    }

    return qt_result;
}