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
extern std::mutex text_mutex;
extern std::string last_text;

extern std::queue<std::vector<int16_t>> audio_queue;
extern std::mutex queue_mutex;
extern std::condition_variable queue_cv;
extern std::atomic<bool> stop_flag;
extern std::atomic<bool> reset_request;

// Функции потоков
void recognition_thread(VoskRecognizer* recognizer);
void output_thread();
void audio_record_thread(pv_recorder_t* recorder);

// Управление распознаванием
void start_speech_recognition(pv_recorder_t* recorder, 
                              VoskRecognizer* recognizer_ru, 
                              VoskRecognizer* recognizer_en = nullptr);
void stop_speech_recognition();

#endif // SPEECH_RECOGNITION_H