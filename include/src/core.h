#ifndef CORE_H_
#define CORE_H_

#include <string>
#include <cctype>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <sstream>
#include <vector>

// Перемещаем реализацию шаблонной функции в заголовок
template <typename T>
T randomizer(std::initializer_list<T> options) {
    if (options.size() == 0) {
        throw std::invalid_argument("randomizer requires non-empty initializer_list");
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, options.size() - 1);

    auto it = options.begin();
    std::advance(it, dist(gen));
    return *it;
}

std::string toLower(const std::string& input);
std::string get_word(const std::string& str, int n);

#endif // CORE_H_