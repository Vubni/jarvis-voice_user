#include "settings.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
    json global_settings;
    std::mutex settings_mutex;
    std::string current_file_path = "settings.json";
}

const json& get_settings() {
    std::lock_guard<std::mutex> lock(settings_mutex);
    return global_settings;
}

void load_settings(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(settings_mutex);
    
    try {
        current_file_path = file_path;
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Settings file not found. Using defaults.");
        }
        file >> global_settings;
    } catch (const std::exception& e) {
        std::cerr << "Settings load error: " << e.what() << "\n";
        // Создаём настройки по умолчанию
        global_settings = json::object();
    }
}

void update_settings(const json& new_settings) {
    std::lock_guard<std::mutex> lock(settings_mutex);
    global_settings.update(new_settings);  // Рекурсивное обновление
}

void save_settings(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(settings_mutex);
    const std::string& path = file_path.empty() ? current_file_path : file_path;
    
    try {
        std::ofstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Can't open file for writing");
        }
        file << global_settings.dump(4);  // Красивый вывод с отступами
    } catch (const std::exception& e) {
        std::cerr << "Settings save failed: " << e.what() << "\n";
    }
}