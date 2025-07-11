#include "logger.h"

std::ofstream error_log_file;

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

    std::cout << log_message;
    if (error_log_file.is_open()) {
        error_log_file << log_message;
        error_log_file.flush(); 
    }
}