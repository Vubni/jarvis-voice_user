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
#include "command_processor.h"
#include "http_client.h"
#include <chrono>
#include <string>
#include <sstream>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include "whisper.h"
#include <algorithm>
#include "sndfile.h"

using namespace std;
using namespace std::chrono;
using json = nlohmann::json;

ofstream error_log_file;

void log_error(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char timestamp_str[20];
    std::strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));

    std::string log_message = "[" + std::string(timestamp_str) + "] ERROR: " + message + "\n";

    std::cerr << log_message;
    if (error_log_file.is_open()) {
        error_log_file << log_message;
        error_log_file.flush(); 
    }
}

void log_info(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char timestamp_str[20];
    std::strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));

    std::string log_message = "[" + std::string(timestamp_str) + "] INFO: " + message + "\n";

    std::cout << log_message; // Используем cout вместо cerr
    if (error_log_file.is_open()) {
        error_log_file << log_message;
        error_log_file.flush(); 
    }
}

string recognized_text;
mutex text_mutex;
string last_text;

queue<vector<int16_t>> audio_queue;
mutex queue_mutex;
condition_variable queue_cv;
atomic<bool> stop_flag(false);
atomic<bool> reset_request(false);

bool containsWord(const string& text, const string& word) {
    string lower_text = text;
    transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
    string lower_word = word;
    transform(lower_word.begin(), lower_word.end(), lower_word.begin(), ::tolower);
    
    stringstream stream(lower_text);
    string w;
    while (stream >> w) {
        // Очистка от пунктуации
        int start = 0, end = w.size();
        while (start < end && ispunct(w[start])) ++start;
        while (end > start && ispunct(w[end-1])) --end;
        string cleaned = w.substr(start, end - start);
        
        if (cleaned == lower_word)
            return true;
    }
    return false;
}

void command_execution(){
    log_info("Получено: " + last_text);
}

void save_audio(const vector<int16_t>& audio_data, const string& filename) {
    if (audio_data.empty()) {
        log_error("Попытка сохранить пустой аудиофайл");
        return;
    }

    SF_INFO info{};
    info.channels = 1;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    info.samplerate = 16000;
    info.frames = audio_data.size();

    SNDFILE* file = sf_open(filename.c_str(), SFM_WRITE, &info);
    if (!file) {
        log_error(string("Не удалось открыть файл для записи: ") + filename + 
                 ". Ошибка: " + sf_strerror(nullptr));
        return;
    }

    sf_count_t written = sf_write_short(file, audio_data.data(), audio_data.size());
    if (written != audio_data.size()) {
        log_error(string("Ошибка записи в файл: записано ") + to_string(written) + 
                 " из " + to_string(audio_data.size()) + " семплов");
    }

    sf_close(file); // Важно закрыть файл для завершения записи
    log_info(string("Сохранено аудио в файл: ") + filename + 
            " (" + to_string(audio_data.size() / 16000.0) + " секунд)");
}


struct whisper_context* whisper_ctx = nullptr;
whisper_full_params wparams;

// Преобразование int16_t в float
vector<float> convert_to_float(const vector<int16_t>& audio_data) {
    vector<float> result(audio_data.size());
    for (size_t i = 0; i < audio_data.size(); ++i) {
        result[i] = static_cast<float>(audio_data[i]) / 32767.0f;
    }
    return result;
}

void recognition_thread() {
    vector<float> accumulated_audio;
    vector<vector<float>> recent_segments; // Хранение целых фрагментов
    const size_t MIN_AUDIO_LENGTH = 16000;  // Минимум 1 секунда
    const size_t MAX_AUDIO_LENGTH = 16000 * 9; // Максимум 9 секунд
    steady_clock::time_point last_recognition_time = steady_clock::now();
    vector<int16_t> audio_buffer_for_saving;

    while (!stop_flag) {
        vector<int16_t> audio_data;
        {
            unique_lock<mutex> lock(queue_mutex);
            queue_cv.wait(lock, [&]{ 
                return !audio_queue.empty() || stop_flag || reset_request; 
            });
            if (stop_flag && audio_queue.empty()) return;
            if (reset_request) {
                accumulated_audio.clear();
                recent_segments.clear(); // Очистка сегментов при сбросе
                last_recognition_time = steady_clock::now();
                continue;
            }
            if (!audio_queue.empty()) {
                audio_data = move(audio_queue.front());
                audio_queue.pop();
            }
        }

        // Сохранение для записи в файл
        audio_buffer_for_saving.insert(audio_buffer_for_saving.end(), 
                                     audio_data.begin(), audio_data.end());

        if (!audio_data.empty() && whisper_ctx) {
            // Преобразование в float и добавление как нового сегмента
            vector<float> float_audio = convert_to_float(audio_data);
            
            // Добавление нового сегмента
            recent_segments.push_back(float_audio);
            accumulated_audio.insert(accumulated_audio.end(), 
                                   float_audio.begin(), float_audio.end());

            // Проверка превышения максимальной длины
            size_t total_size = accumulated_audio.size();
            if (total_size > MAX_AUDIO_LENGTH) {
                // Удаление старых сегментов до тех пор, пока не уложимся в лимит
                while (!recent_segments.empty() && total_size > MAX_AUDIO_LENGTH) {
                    // Удаляем самый старый сегмент
                    total_size -= recent_segments.front().size();
                    recent_segments.erase(recent_segments.begin());
                }

                // Пересобираем accumulated_audio из оставшихся сегментов
                accumulated_audio.clear();
                for (const auto& segment : recent_segments) {
                    accumulated_audio.insert(accumulated_audio.end(), 
                                            segment.begin(), segment.end());
                }
            }

            // Проверка условий для распознавания
            auto now = steady_clock::now();
            auto elapsed_ms = duration_cast<milliseconds>(now - last_recognition_time).count();
            if (elapsed_ms >= 200 && accumulated_audio.size() >= MIN_AUDIO_LENGTH) {
                // Распознавание
                if (whisper_full(whisper_ctx, wparams, accumulated_audio.data(), 
                                accumulated_audio.size()) == 0) {
                    int n_segments = whisper_full_n_segments(whisper_ctx);
                    if (n_segments > 0) {
                        const char* text = whisper_full_get_segment_text(whisper_ctx, 
                                                                   n_segments - 1);
                        if (text && string(text) != recognized_text) {
                            lock_guard<mutex> lock(text_mutex);
                            recognized_text = text;
                        }
                    }
                }
                last_recognition_time = now;
            }
        }
    }
}

void output_thread() {
    time_point<high_resolution_clock> last_change_time = high_resolution_clock::now();

    while (!stop_flag) {
        this_thread::sleep_for(chrono::milliseconds(100));
        lock_guard<mutex> lock(text_mutex);

        if (recognized_text != last_text) {
            // Обновляем текст на экране
            cout << "\r\33[2K[Распознавание]: " << recognized_text << flush;
            last_text = recognized_text;

            last_change_time = high_resolution_clock::now();
        }

        // Проверяем, прошло ли достаточно времени с последнего изменения текста
        auto now = high_resolution_clock::now();
        auto elapsed_ms = duration_cast<milliseconds>(now - last_change_time).count();
        const int silence_timeout = 500;

        if (elapsed_ms >= silence_timeout && !last_text.empty()) {
            if (containsWord(last_text, "джарвис")) {
                command_execution();
            }
            cout << "\r\33[2K[Распознавание]: " << flush; // Принудительная очистка строки
        }
    }
}

int main() {
    // Установка кодировки UTF-8 для Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Создание папки logs, если её нет
    if (!filesystem::exists("logs")) {
        filesystem::create_directory("logs");
    }

    error_log_file.open("logs/error.log", ios_base::app); // Добавляем к предыдущим логам
    if (!error_log_file.is_open()) {
        cerr << "Не удалось открыть файл логов!" << endl;
    } else {
        log_info("Программа запущена. Логирование начато.");
    }
    
    whisper_context_params cparams = whisper_context_default_params();
    cparams.use_gpu = true;
    cparams.flash_attn = false;
    whisper_ctx = whisper_init_from_file_with_params("models/base.bin", cparams);
    if (!whisper_ctx) {
        cerr << "Не удалось инициализировать модель Whisper." << endl;
        return -1;
    }
    
    // Настройка параметров распознавания
    wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.language = "ru";         // Русский язык
    wparams.n_threads = 12; // Ограничьте количество потоков CPU
    wparams.beam_search.beam_size = 1;
    wparams.print_realtime = false;
    wparams.print_progress = false;
    wparams.print_timestamps = false;
    wparams.print_special = false;

    // Инициализация рекордера
    pv_recorder_t* recorder = nullptr;
    if (pv_recorder_init(512, -1, 5, &recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "Ошибка инициализации PvRecorder" << endl;
        whisper_free(whisper_ctx);
        return -1;
    }
    
    if (pv_recorder_start(recorder) != PV_RECORDER_STATUS_SUCCESS) {
        cerr << "Ошибка запуска записи" << endl;
        pv_recorder_delete(recorder);
        whisper_free(whisper_ctx);
        return -1;
    }
    
    cout << "Запись началась... Говорите в микрофон. Нажмите Ctrl+C для выхода." << endl;

    // Запуск потока распознавания
    thread rec_thread;
    thread out_thread;
    try {
        rec_thread = thread(recognition_thread);
        out_thread = thread(output_thread);
    } catch (const exception& e) {
        cerr << "Ошибка создания потока: " << e.what() << endl;
        pv_recorder_stop(recorder);
        pv_recorder_delete(recorder);
        whisper_free(whisper_ctx);
        return -1;
    }

    // Чтение аудио в основном потоке
    while (!stop_flag) {
        vector<int16_t> audio_buffer(512);
        
        if (pv_recorder_read(recorder, audio_buffer.data()) == PV_RECORDER_STATUS_SUCCESS) {
            // Добавляем данные в очередь
            {
                lock_guard<mutex> lock(queue_mutex);
                audio_queue.push(audio_buffer);
            }
            queue_cv.notify_one();
        }
        
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    if (rec_thread.joinable()) rec_thread.join();
    if (out_thread.joinable()) out_thread.join();

    // Очистка
    pv_recorder_stop(recorder);
    pv_recorder_delete(recorder);
    whisper_free(whisper_ctx);
    
    if (error_log_file.is_open()) {
        log_error("Программа завершена.");
        error_log_file.close();
    }
    
    return 0;
}