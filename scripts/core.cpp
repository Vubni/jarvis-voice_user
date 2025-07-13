#include "core.h"
using namespace std;

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