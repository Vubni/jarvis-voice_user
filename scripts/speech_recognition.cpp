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
std::string recognized_text;
std::mutex text_mutex;
std::string last_text;

std::queue<std::vector<int16_t>> audio_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;
std::atomic<bool> stop_flag(false);
std::atomic<bool> reset_request(false);

// Внешние зависимости
extern AnimationController controller;

void recognition_thread(VoskRecognizer* recognizer) {
    while (!stop_flag) {
        vector<int16_t> audio_data;
        try {
            unique_lock<mutex> lock(queue_mutex);
            queue_cv.wait(lock, [&]{ 
                return !audio_queue.empty() || stop_flag || reset_request; 
            });

            if (stop_flag && audio_queue.empty()) return;

            if (reset_request) {
                vosk_recognizer_reset(recognizer);
                reset_request = false;
                
                queue<vector<int16_t>> empty_queue;
                audio_queue.swap(empty_queue);

                recognized_text.clear();
                last_text.clear();
                continue;
            }

            if (!audio_queue.empty()) {
                audio_data = move(audio_queue.front());
                audio_queue.pop();
            }
        } catch (const exception& e){
            log_error("Recognition thread error: " + string(e.what()));
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
                log_error("JSON parsing error: " + string(e.what()));
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

    while (!stop_flag) {
        this_thread::sleep_for(chrono::milliseconds(100));
        lock_guard<mutex> lock(text_mutex);

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
            if (elapsed >= 5000 && last_text == jarvis_text) play_status = false;
            if (last_text == jarvis_text) continue;
        }

        auto elapsed_ms = duration_cast<milliseconds>(now - last_change_time).count();
        const int silence_timeout = 800;
        auto elapsed_ms_text = duration_cast<milliseconds>(now - last_change_text_time).count();

        if (elapsed_ms >= silence_timeout && !last_text.empty() && elapsed_ms_text >= 700) {
            if ((last_text.find("джарв") != string::npos && get_word(last_text, -1).find("джарв") == string::npos) || last_text.find("джерв") != string::npos && get_word(last_text, -1).find("джерв") == string::npos) {
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
            this_thread::sleep_for(95ms);  // Исправлено: стандартная задержка
            recognized_text.clear();
            last_text.clear();
        }
    }
}


void audio_record_thread(pv_recorder_t* recorder) {
    while (!stop_flag) {
        vector<int16_t> audio_buffer(512);
        if (pv_recorder_read(recorder, audio_buffer.data()) == PV_RECORDER_STATUS_SUCCESS) {
            {
                lock_guard<mutex> lock(queue_mutex);
                audio_queue.push(audio_buffer);
            }
            queue_cv.notify_one();
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void start_speech_recognition(VoskRecognizer* recognizer, pv_recorder_t* recorder) {
    thread audio_thread(audio_record_thread, recorder);
    thread rec_thread(recognition_thread, recognizer);
    thread out_thread(output_thread);

    audio_thread.detach();
    rec_thread.detach();
    out_thread.detach();
}

void stop_speech_recognition() {
    stop_flag = true;
    queue_cv.notify_all();
}