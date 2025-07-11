#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <fstream>
#include <windows.h>
#include <nlohmann/json.hpp>
#include "pv_recorder.h"
#include "vosk_api.h"
#include <chrono>
#include <string>
#include <sstream>
#include <cctype>
#include <filesystem>
#include <iomanip>

#include "logger.h"
#include "command_processor.h"
#include "http_client.h"
#include "api.h"
#include "my_commands.h"
#include "core.h"
#include "audioPlayer.h"
#include "audioControl.h"

#include <QApplication>
#include <QThread>
#include "gradientwidget.h"
#include "animationcontroller.h"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

string recognized_text;
mutex text_mutex;
string last_text;

queue<vector<int16_t>> audio_queue;
mutex queue_mutex;
condition_variable queue_cv;
atomic<bool> stop_flag(false);
atomic<bool> reset_request(false);

AnimationController controller;


void recognition_thread(VoskRecognizer* recognizer) {
    while (!stop_flag) {
        vector<int16_t> audio_data;
        try {
            unique_lock<mutex> lock(queue_mutex);
            queue_cv.wait(lock, [&]{ 
                return !audio_queue.empty() || stop_flag || reset_request; 
            });

            if (stop_flag && audio_queue.empty()) return;

            // Обработка сброса
            if (reset_request) {
                vosk_recognizer_reset(recognizer);
                reset_request = false;
                
                // Очистка очереди аудиоданных
                queue<vector<int16_t>> empty_queue;
                audio_queue.swap(empty_queue);

                recognized_text.clear();
                last_text.clear();
                
                continue;  // Продолжить цикл без обработки данных
            }

            if (!audio_queue.empty()) {
                audio_data = move(audio_queue.front());
                audio_queue.pop();
            } }
        catch (const exception& e){
            log_error("Ошибка очистки потока" + string(e.what()));
        }

        if (!audio_data.empty()) {
            int num_samples = static_cast<int>(audio_data.size());
            vosk_recognizer_accept_waveform_s(recognizer, audio_data.data(), num_samples);

            const char* partial_result = vosk_recognizer_partial_result(recognizer);
            try {
                json j = json::parse(partial_result);
                string text = j["partial"];

                if (!text.empty()) {
                    lock_guard<mutex> lock(text_mutex);
                    recognized_text = text;
                }
            } catch (const exception& e) {
                log_error("Ошибка разбора JSON: " + string(e.what()));
            }
        }
    }
}

void output_thread() {
    time_point<high_resolution_clock> last_change_time = high_resolution_clock::now();
    time_point<high_resolution_clock> last_change_text_time = high_resolution_clock::now();
    time_point<high_resolution_clock> last_found_jarvis;
    bool play_pending = false;
    bool play_status = false;
    string pending_audio_path;
    string jarvis_text;
    bool status = true;

    while (!stop_flag) {
        this_thread::sleep_for(chrono::milliseconds(100));
        lock_guard<mutex> lock(text_mutex);

        if (recognized_text != last_text) {
            cout << "\r\33[2K[Распознавание]: " << recognized_text << flush;
            last_text = recognized_text;
            last_change_time = high_resolution_clock::now();
            last_change_text_time = high_resolution_clock::now();  // Обновляем при любом изменении текста

            if (status && last_text.find("джарв") != string::npos) {
                MuteOtherApplications();
                QMetaObject::invokeMethod(&controller, "toggleAnimation", Qt::QueuedConnection, 
                             Q_ARG(bool, true));
                if (get_word(last_text, -1).find("джарв") != string::npos){
                    play_pending = true;
                }
                play_status = false;
                status = false;
                last_found_jarvis = high_resolution_clock::now();
                jarvis_text = last_text;
                cout << endl << jarvis_text << endl;
                pending_audio_path = "greet/" + (string)randomizer({"1", "2"});
            } else if (last_text.find("джарв") == string::npos && play_pending) {
                play_pending = false;
            }
        }

        auto now = high_resolution_clock::now();
        if (play_pending) {
            auto elapsed = duration_cast<milliseconds>(now - last_found_jarvis).count();
            if (elapsed >= 350 && last_text == jarvis_text) {
                cout << endl << jarvis_text << endl;
                thread(playAudio, pending_audio_path).detach();
                play_pending = false;
                play_status = true;
            } else if (last_text != jarvis_text || get_word(last_text, -1).find("джарв") == string::npos) {
                play_pending = false;
            }
        }

        if (play_status){
            auto elapsed = duration_cast<milliseconds>(now - last_found_jarvis).count();
            if (elapsed >= 5000 && last_text == jarvis_text) play_status = false;
            if (last_text == jarvis_text) continue;
        }

        auto elapsed_ms = duration_cast<milliseconds>(now - last_change_time).count();
        const int silence_timeout = 800;
        auto elapsed_ms_text = duration_cast<milliseconds>(now - last_change_text_time).count();

        if (elapsed_ms >= silence_timeout && !last_text.empty() && elapsed_ms_text >= 700) {
            if (last_text.find("джарв") != string::npos && get_word(last_text, -1).find("джарв") == string::npos) {
                // Вызов команды
                cout << endl << last_text << endl;
                thread execution = thread(command_execution, last_text);
                execution.detach();
            }

            status = true;
            play_status = false;
            reset_request = true;
            queue_cv.notify_one();
            RestoreApplicationVolumes();
            QMetaObject::invokeMethod(&controller, "toggleAnimation", Qt::QueuedConnection, 
                             Q_ARG(bool, false));
            Sleep(95);
            recognized_text.clear();
            last_text.clear();
        }
    }
}

// Поток для записи аудио
void audio_record_thread(pv_recorder_t* recorder) {
    int file_counter = 0;
    while (!stop_flag) {
        vector<int16_t> audio_buffer(512);
        if (pv_recorder_read(recorder, audio_buffer.data()) == PV_RECORDER_STATUS_SUCCESS) {
            // Сохраняем каждый буфер в отдельный файл (для отладки/аналитики)
            stringstream filename;
            filename << "audio_chunks/chunk_" << setw(5) << setfill('0') << file_counter++ << ".raw";
            ofstream out(filename.str(), ios::binary);
            if (out.is_open()) {
                out.write(reinterpret_cast<const char*>(audio_buffer.data()), audio_buffer.size() * sizeof(int16_t));
                out.close();
            }
            // Добавляем данные в очередь для распознавания
            {
                lock_guard<mutex> lock(queue_mutex);
                audio_queue.push(audio_buffer);
            }
            queue_cv.notify_one();
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Создаем контроллер анимации
    QThread workerThread;
    controller.moveToThread(&workerThread);
    QObject::connect(&controller, &AnimationController::toggleAnimation, 
                    [&](bool isActive) {
        if (isActive) {
            OnAnimation();
        } else {
            OffAnimation();
        }
    });
    workerThread.start();

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (!filesystem::exists("logs")) {
        filesystem::create_directory("logs");
    }
    if (!filesystem::exists("audio_chunks")) {
        filesystem::create_directory("audio_chunks");
    }

    error_log_file.open("logs/error.log", ios_base::app);
    if (!error_log_file.is_open()) {
        cerr << "Не удалось открыть файл логов!" << endl;
    } else {
        log_info("Программа запущена. Логирование начато.");
    }

    mainCommands();

    VoskModel* model = vosk_model_new("./models/vosk-big");
    if (!model) {
        cerr << "Не удалось загрузить модель Vosk" << endl;
        return -1;
    }

    VoskRecognizer* recognizer = vosk_recognizer_new(model, 16000.0f);
    vosk_recognizer_set_max_alternatives(recognizer, 0);

    pv_recorder_t* recorder = nullptr;
    if (pv_recorder_init(512, -1, 5, &recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "Ошибка инициализации PvRecorder" << endl;
        vosk_model_free(model);
        return -1;
    }
    if (pv_recorder_start(recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "Ошибка запуска записи" << endl;
        pv_recorder_delete(recorder);
        vosk_model_free(model);
        return -1;
    }

    cout << "Запись началась... Говорите в микрофон. Нажмите Ctrl+C для выхода." << endl;

    thread rec_thread;
    thread out_thread;
    thread audio_thread;
    bool thread_started = false;
    try {
        audio_thread = thread(audio_record_thread, recorder);
        rec_thread = thread(recognition_thread, recognizer);
        out_thread = thread(output_thread);
        thread_started = true;
    } catch (const exception& e) {
        cerr << "Ошибка создания потока: " << e.what() << endl;
        pv_recorder_stop(recorder);
        pv_recorder_delete(recorder);
        vosk_recognizer_free(recognizer);
        vosk_model_free(model);
        return -1;
    }

    // Главный поток — только Qt event loop
    int qt_result = app.exec();

    // Завершаем работу потоков
    stop_flag = true;
    queue_cv.notify_all();
    if (audio_thread.joinable()) audio_thread.join();
    if (rec_thread.joinable()) rec_thread.join();
    if (out_thread.joinable()) out_thread.join();

    pv_recorder_stop(recorder);
    pv_recorder_delete(recorder);
    vosk_recognizer_free(recognizer);
    vosk_model_free(model);

    if (error_log_file.is_open()) {
        log_error("Программа завершена.");
        error_log_file.close();
    }

    return qt_result;
}