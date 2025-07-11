#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>

// Глобальная переменная для лог-файла (определение будет в .cpp файле)
extern std::ofstream error_log_file;

// Функция записи ошибок в лог
void log_error(const std::string& message);

// Функция записи информационных сообщений в лог
void log_info(const std::string& message);

#endif // LOGGER_H