#include "core.h"
#include <algorithm>
#include <cctype>
using namespace std;

std::string toLowerCase(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length();) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        // Обработка ASCII символов
        if (c < 0x80) {
            result += static_cast<char>(std::tolower(c));
            i++;
        }
        // Обработка двухбайтовых символов UTF-8 (кириллица)
        else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= str.length()) {
                result += c; // Неполный символ, копируем как есть
                break;
            }
            unsigned char c1 = static_cast<unsigned char>(str[i + 1]);
            // Декодируем символ в кодовую точку Unicode
            uint32_t code_point = ((c & 0x1F) << 6) | (c1 & 0x3F);
            
            // Преобразуем русские заглавные буквы в строчные
            if (code_point == 0x0401) { // Буква Ё
                code_point = 0x0451;   // Заменяем на ё
            } else if (code_point >= 0x0410 && code_point <= 0x042F) {
                code_point += 0x20; // Преобразуем А-Я в а-я
            }
            
            // Кодируем обратно в UTF-8
            result += static_cast<char>(0xC0 | (code_point >> 6));
            result += static_cast<char>(0x80 | (code_point & 0x3F));
            i += 2;
        }
        // Пропускаем символы длиннее 2 байт без изменений
        else if ((c & 0xF0) == 0xE0) { // 3-байтовые символы
            if (i + 2 >= str.length()) break;
            result.append(str.substr(i, 3));
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-байтовые символы
            if (i + 3 >= str.length()) break;
            result.append(str.substr(i, 4));
            i += 4;
        } else {
            result += c; // Некорректный байт, оставляем как есть
            i++;
        }
    }
    return result;
}

// Заменяет все вхождения подстроки oldChar на newChar (работает с UTF-8)
void replaceChar(std::string& str, const std::string& oldChar, const std::string& newChar) {
    size_t pos = 0;
    while ((pos = str.find(oldChar, pos)) != std::string::npos) {
        str.replace(pos, oldChar.length(), newChar);
        pos += newChar.length();
    }
}

// Удаляет n символов с конца строки (корректно для UTF-8)
void truncateFromEnd(std::string& str, size_t n) {
    if (n == 0 || str.empty()) return;
    
    size_t count = 0;
    size_t pos = str.length();
    
    while (pos > 0) {
        pos--;
        unsigned char c = static_cast<unsigned char>(str[pos]);
        if ((c & 0xC0) != 0x80) { // Начало символа UTF-8
            count++;
            if (count == n) break;
        }
    }
    
    str.resize(pos);
}

string json_to_key_value_string(const json& data) {
    std::string result;
    
    if (data.is_object()) {
        for (auto& el : data.items()) {
            // Обработка значения в зависимости от типа
            std::string value_str;
            if (el.value().is_string()) {
                value_str = el.value().get<std::string>();
            } else {
                value_str = el.value().dump();
            }
            result += el.key() + " | " + value_str + "\n";
        }
    } 
    else if (data.is_array()) {
        for (size_t i = 0; i < data.size(); ++i) {
            std::string value_str;
            if (data[i].is_string()) {
                value_str = data[i].get<std::string>();
            } else {
                value_str = data[i].dump();
            }
            result += std::to_string(i) + " | " + value_str + "\n";
        }
    } 
    else {
        // Обработка примитивных типов
        if (data.is_string()) {
            result = "value | " + data.get<std::string>();
        } else {
            result = "value | " + data.dump();
        }
    }
    
    return result;
}

std::string toLower(const std::string& input) {
    std::string result;
    result.reserve(input.size());

    for (char c : input) {
        result += static_cast<char>(
            std::tolower(static_cast<unsigned char>(c))
        );
    }

    return result;
}

bool containsWord(const string& text, const string& word) {
    stringstream stream(text);
    string w;
    while (stream >> w) {
        int start = 0;
        while (start < w.size() && ispunct(w[start])) ++start;
        
        int end = w.size();
        while (end > start && ispunct(w[end - 1])) --end;
        
        string cleaned = w.substr(start, end - start);
        
        if (cleaned == word)
            return true;
    }
    return false;
}

std::string get_word(const std::string& str, int n) {
    std::istringstream iss(str);
    std::vector<std::string> words;
    std::string word;

    // Разбиваем строку на слова
    while (iss >> word) {
        words.push_back(word);
    }

    // Проверяем, что есть слова
    if (words.empty()) {
        return "";
    }

    // Определяем индекс
    int index;
    if (n > 0) {
        index = n - 1;
        if (index < 0 || index >= static_cast<int>(words.size())) {
            return "";
        }
    } else if (n < 0) {
        index = static_cast<int>(words.size()) + n;
        if (index < 0 || index >= static_cast<int>(words.size())) {
            return "";
        }
    } else {
        // n == 0 — недопустимый индекс
        return "";
    }

    return words[index];
}