#include "speech_recognition.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include "logger.h"
#include "command_processor.h"
#include "audioControl.h"
#include "core.h"
#include "animationcontroller.h"
#include "audioPlayer.h"
#include "api.h"
#include <Windows.h>

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

// Определение глобальных переменных
std::string recognized_text;      // Русский текст
std::string recognized_text_en;   // Английский текст
std::mutex text_mutex;
std::string last_text;

std::queue<std::vector<int16_t>> audio_queue_ru;
std::queue<std::vector<int16_t>> audio_queue_en;
std::mutex queue_ru_mutex;
std::mutex queue_en_mutex;
std::condition_variable queue_ru_cv;
std::condition_variable queue_en_cv;
std::atomic<bool> stop_flag(false);
std::atomic<bool> reset_request(false);
std::atomic<bool> reset_request_en(false);

// Внешние зависимости
extern AnimationController controller;

void recognition_thread(VoskRecognizer* recognizer) {
    while (!stop_flag) {
        vector<int16_t> audio_data;
        try {
            unique_lock<mutex> lock(queue_ru_mutex);
            queue_ru_cv.wait(lock, [&]{ 
                return !audio_queue_ru.empty() || stop_flag || reset_request; 
            });

            if (stop_flag) return;
            if (reset_request) {
                vosk_recognizer_reset(recognizer);
                audio_queue_ru = {};

                recognized_text.clear();
                last_text.clear();
                reset_request=false;
                continue;
            }

            if (!audio_queue_ru.empty()) {
                audio_data = std::move(audio_queue_ru.front());
                audio_queue_ru.pop();
            }
        } catch (const exception& e){
            log_error("RU recognition error: " + string(e.what()));
        }

        // Обработка аудиоданных
        if (!audio_data.empty()) {
            vosk_recognizer_accept_waveform_s(recognizer, audio_data.data(), static_cast<int>(audio_data.size()));
            
            const char* partial = vosk_recognizer_partial_result(recognizer);
            if (partial != nullptr) {
                try {
                    lock_guard<mutex> lock(text_mutex);
                    recognized_text = json::parse(partial)["partial"];
                } catch (const exception& e) {
                    log_error("RU JSON error: " + string(e.what()));
                }
            }
        }
    }
}

void recognition_en_thread(VoskRecognizer* recognizer) {
    if (!recognizer) {
        log_error("EN recognizer is null!");
        return;
    }
    while (!stop_flag) {
        vector<int16_t> audio_data;
        try {
            unique_lock<mutex> lock(queue_en_mutex);
            queue_en_cv.wait(lock, [&]{ 
                return !audio_queue_en.empty() || stop_flag || reset_request; 
            });

            if (stop_flag) return;
            if (reset_request_en) {
                vosk_recognizer_reset(recognizer);
                audio_queue_en = {};

                recognized_text_en.clear();
                reset_request_en=false;
                continue;
            }

            if (!audio_queue_en.empty()) {
                audio_data = std::move(audio_queue_en.front());
                audio_queue_en.pop();
            }
        } catch (const exception& e){
            log_error("EN recognition error: " + string(e.what()));
        }

        // Обработка аудиоданных
        if (!audio_data.empty()) {
            vosk_recognizer_accept_waveform_s(recognizer, audio_data.data(), static_cast<int>(audio_data.size()));
            
            const char* partial = vosk_recognizer_partial_result(recognizer);
            if (partial != nullptr) {
                try {
                    lock_guard<mutex> lock(text_mutex);
                    recognized_text_en = json::parse(partial)["partial"];
                } catch (const exception& e) {
                    log_error("EN JSON error: " + string(e.what()));
                }
            }
        }
    }
}



bool contains_jarvis(const string& text) {
    return text.find("джарв") != string::npos || text.find("джерв") != string::npos;
}

bool is_last_word_jarvis(const string& text) {
    string last_word = get_word(text, -1);
    return last_word.find("джарв") != string::npos || last_word.find("джерв") != string::npos;
}

void output_thread() {
    time_point<system_clock> last_change_time = system_clock::now();
    time_point<system_clock> last_change_text_time = system_clock::now();
    time_point<system_clock> last_found_jarvis;
    bool play_pending = false;
    bool play_status = false;
    string pending_audio_path;
    string jarvis_text;
    bool status = true;
    int count_attemp_jarvis = 0;

    while (!stop_flag) {
        this_thread::sleep_for(chrono::milliseconds(100));
        lock_guard<mutex> lock(text_mutex);

        if (recognized_text.empty() && recognized_text_en.empty()) {
            count_attemp_jarvis++;
            if (count_attemp_jarvis >= 15) {
                count_attemp_jarvis = 0;
                reset_request = true;
                reset_request_en = true;
                queue_ru_cv.notify_all();
                queue_en_cv.notify_all();
            }
        }

        if (recognized_text != last_text) {
            cout << "\r\33[2K[Recognition]: " << recognized_text << flush;
            last_text = recognized_text;
            last_change_time = system_clock::now();
            last_change_text_time = system_clock::now();

            if (status && contains_jarvis(last_text)) {
                MuteOtherApplications();
                QMetaObject::invokeMethod(&controller, "toggleAnimation", Qt::QueuedConnection, 
                             Q_ARG(bool, true));
                if (is_last_word_jarvis(last_text)) {
                    play_pending = true;
                    pending_audio_path = "greet/" + (string)randomizer({"1", "2"});
                }
                
                last_found_jarvis = system_clock::now();
                jarvis_text = last_text; 
                play_status = false;
                status = false;
                cout << endl << jarvis_text << endl;
            }
        }

        auto now = system_clock::now();
        if (play_pending) {
            auto elapsed = duration_cast<milliseconds>(now - last_found_jarvis).count();
            if (elapsed >= 350 && last_text == jarvis_text) {
                thread(playAudio, pending_audio_path).detach();
                play_pending = false;
                play_status = true;
            } else if (!is_last_word_jarvis(last_text)) {
                play_pending = false;
            }
        }

        if (play_status){
            auto elapsed = duration_cast<milliseconds>(now - last_found_jarvis).count();
            if (elapsed >= 5500 && last_text == jarvis_text) play_status = false;
            if (last_text == jarvis_text) continue;
        }

        auto elapsed_ms = duration_cast<milliseconds>(now - last_change_time).count();
        const int silence_timeout = 800;
        auto elapsed_ms_text = duration_cast<milliseconds>(now - last_change_text_time).count();

        if (elapsed_ms >= silence_timeout && !last_text.empty() && elapsed_ms_text >= 850) {
            bool status_execution = false;
            if (contains_jarvis(last_text) && !is_last_word_jarvis(last_text)) {
                try{
                    status_execution = command_execution(last_text, recognized_text_en);
                } catch (const exception& e) {
                    log_error("Command execution error: " + (string)e.what());
                }
            }

            status = true;
            play_status = false;
            reset_request = true;
            reset_request_en = true;
            // Нотифицируем потоки распознавания
            queue_ru_cv.notify_all();
            queue_en_cv.notify_all();
            recognized_text.clear();
            last_text.clear();

            RestoreApplicationVolumes();
            this_thread::sleep_for(100ms);
            QMetaObject::invokeMethod(&controller, "toggleAnimation", Qt::QueuedConnection, 
                            Q_ARG(bool, false));
        }
    }
}


void audio_record_thread(pv_recorder_t* recorder) {
    while (!stop_flag) {
        vector<int16_t> audio_buffer(512);
        if (pv_recorder_read(recorder, audio_buffer.data()) == PV_RECORDER_STATUS_SUCCESS) {
            // Отправка в обе очереди
            {
                lock_guard<mutex> lock_ru(queue_ru_mutex);
                audio_queue_ru.push(audio_buffer);
            }
            queue_ru_cv.notify_one();
            
            {
                lock_guard<mutex> lock_en(queue_en_mutex);
                audio_queue_en.push(audio_buffer);
            }
            queue_en_cv.notify_one();
        }
        this_thread::sleep_for(10ms);
    }
}
void start_speech_recognition(pv_recorder_t* recorder, 
                              VoskRecognizer* recognizer_ru, 
                              VoskRecognizer* recognizer_en) 
{
    thread(audio_record_thread, recorder).detach();
    thread(recognition_thread, recognizer_ru).detach();
    
    if (recognizer_en) {
        thread(recognition_en_thread, recognizer_en).detach();
    }
    thread(output_thread).detach();
}

void stop_speech_recognition() {
    stop_flag = true;
    queue_ru_cv.notify_all();
    queue_en_cv.notify_all();
}