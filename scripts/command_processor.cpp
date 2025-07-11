#include "command_processor.h"
#include <cctype>

ScriptEngine::ScriptEngine() {}
ScriptEngine::~ScriptEngine() {}

void ScriptEngine::Execute(const std::string& script) {
    try {
        auto commands = SplitCommands(script);
        for (const auto& cmd : commands) {
            auto [name, args] = ParseCommand(cmd);
            auto it = m_functions.find(name);
            if (it == m_functions.end()) {
                throw std::runtime_error("Function not found: " + name);
            }
            FunctionBase* func = it->second.get();
            size_t total_params = func->GetTotalParams();
            size_t default_count = func->GetDefaultParamsCount();
            size_t provided = args.size();
            if (provided > total_params) {
                throw std::runtime_error("Too many arguments for function: " + name);
            }
            if (provided < (total_params - default_count)) {
                throw std::runtime_error("Not enough arguments for function: " + name);
            }
            std::vector<std::any> all_args = args;
            const std::vector<std::any>& defaults = func->GetDefaults();
            size_t needed = total_params - provided;
            for (size_t i = 0; i < needed; ++i) {
                if (i < defaults.size()) {
                    all_args.push_back(defaults[i]);
                } else {
                    throw std::runtime_error("Unexpected missing default value");
                }
            }
            func->Call(all_args);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error executing script: " << e.what() << std::endl;
    }
}

std::vector<std::string> ScriptEngine::SplitCommands(const std::string& script) {
    std::vector<std::string> commands;
    std::string::size_type start = 0;
    while (start < script.size()) {
        auto end = script.find(';', start);
        if (end == std::string::npos)
            break;
        std::string command = script.substr(start, end - start);
        if (!command.empty()) {
            commands.push_back(Trim(command));
        }
        start = end + 1;
    }
    // Добавляем оставшуюся часть строки после последней точки с запятой
    if (start < script.size()) {
        std::string command = script.substr(start);
        if (!command.empty()) {
            commands.push_back(Trim(command));
        }
    }
    return commands;
}

std::pair<std::string, std::vector<std::any>> ScriptEngine::ParseCommand(const std::string& command) {
    std::string trimmed = Trim(command);
    
    // Найти позицию открывающей скобки
    size_t open_pos = trimmed.find('(');
    if (open_pos == std::string::npos || open_pos == 0) {
        throw std::runtime_error("Invalid function format: missing '('");
    }
    
    // Извлечь имя функции
    std::string func_name = Trim(trimmed.substr(0, open_pos));
    
    // Проверить, есть ли закрывающая скобка
    size_t close_pos = trimmed.find(')', open_pos);
    if (close_pos == std::string::npos) {
        throw std::runtime_error("Invalid function format: missing ')'");
    }
    
    // Извлечь аргументы
    std::string args_str = trimmed.substr(open_pos + 1, close_pos - open_pos - 1);
    std::vector<std::string> args_list = SplitArgs(args_str);
    
    // Преобразовать аргументы
    std::vector<std::any> result;
    for (const auto& arg : args_list) {
        result.push_back(ConvertArg(arg));
    }
    
    return {func_name, result};
}

std::vector<std::string> ScriptEngine::SplitArgs(const std::string& args_str) {
    std::vector<std::string> args;
    std::string current;
    bool in_quotes = false;
    for (char c : args_str) {
        if (c == '"') {
            in_quotes = !in_quotes;
            current += c;
        } else if (c == ',' && !in_quotes) {
            args.push_back(Trim(current));
            current.clear();
        } else if (!std::isspace(static_cast<unsigned char>(c))) {
            current += c;
        }
    }
    if (!current.empty()) {
        args.push_back(Trim(current));
    }
    return args;
}

std::any ScriptEngine::ConvertArg(const std::string& arg_str) {
    std::string s = Trim(arg_str);
    if (s.empty()) {
        return {};
    }
    if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')) {
        return std::string(s.substr(1, s.size() - 2));
    }
    if (toLower(s) == "true") {
        return true;
    } else if (toLower(s) == "false") {
        return false;
    }
    try {
        return std::stoi(s);
    } catch (const std::exception& e) {
        std::cerr << "Error ConvertArg script: " << e.what() << std::endl;
    }
}

std::string ScriptEngine::Trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        ++start;
    }
    auto end = s.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}