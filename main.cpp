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
#include "gradientwidget.h"
#include "animationcontroller.h"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

AnimationController controller;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QObject::connect(&controller, &AnimationController::toggleAnimation, 
                    [&](bool isActive) {
        if (isActive) {
            OnAnimation();
        } else {
            OffAnimation();
        }
    });

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (!filesystem::exists("logs")) filesystem::create_directory("logs");
    if (!filesystem::exists("audio_chunks")) filesystem::create_directory("audio_chunks");

    error_log_file.open("logs/error.log", ios_base::app);
    if (error_log_file.is_open()) {
        log_info("Application started");
    }

    mainCommands();

    log_info("Create session..");
    nlohmann::json pathsPrograms = InstalledPrograms::GetInstalledPrograms();
    create_session(pathsPrograms);
    log_info("Successful Create session.");

    VoskModel* model = vosk_model_new("./models/small");
    if (!model) {
        cerr << "Failed to load Vosk model" << endl;
        return -1;
    }

    VoskRecognizer* recognizer = vosk_recognizer_new(model, 16000.0f);
    vosk_recognizer_set_max_alternatives(recognizer, 0);

    pv_recorder_t* recorder = nullptr;
    if (pv_recorder_init(512, -1, 5, &recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "PvRecorder init error" << endl;
        vosk_model_free(model);
        return -1;
    }
    if (pv_recorder_start(recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "Recording start error" << endl;
        pv_recorder_delete(recorder);
        vosk_model_free(model);
        return -1;
    }

    cout << "Recording started... Speak into the microphone. Press Ctrl+C to exit." << endl;

    // Запуск потоков распознавания
    start_speech_recognition(recognizer, recorder);

    int qt_result = app.exec();

    // Остановка распознавания и очистка ресурсов
    stop_speech_recognition();
    pv_recorder_stop(recorder);
    pv_recorder_delete(recorder);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);

    if (error_log_file.is_open()) {
        log_error("Application terminated");
        error_log_file.close();
    }

    return qt_result;
}