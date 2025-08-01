#ifndef SPEECH_RECOGNITION_H
#define SPEECH_RECOGNITION_H

#include <pv_recorder.h>
#include "vosk_api.h"
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string>

// Глобальные переменные для управления распознаванием
extern std::string recognized_text;
extern std::string recognized_text_en;
extern std::mutex text_mutex;
extern std::string last_text;

// Исправленные объявления (добавлены _ru и _en суффиксы)
extern std::queue<std::vector<int16_t>> audio_queue_ru;
extern std::queue<std::vector<int16_t>> audio_queue_en;
extern std::mutex queue_ru_mutex;
extern std::mutex queue_en_mutex;
extern std::condition_variable queue_ru_cv;
extern std::condition_variable queue_en_cv;

extern std::atomic<bool> stop_flag;
extern std::atomic<bool> reset_request;
extern std::atomic<bool> reset_request_en; // Добавлено
extern std::atomic<bool> recording_enabled;

extern VoskRecognizer* recognizer_ru;
extern VoskRecognizer* recognizer_en;

// Функции потоков
void output_thread();
void audio_record_thread(pv_recorder_t* recorder);

// Управление распознаванием
void start_speech_recognition(pv_recorder_t* recorder);
void stop_speech_recognition();

#endif // SPEECH_RECOGNITION_H