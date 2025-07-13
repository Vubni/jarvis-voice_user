#include "api.h"
#include <vector>

void create_session(json pathsPrograms) {
    const std::string base_url = "https://api.vubni.com/create_session"; 
    
    // Преобразуем JSON в строку специального формата
    std::string formatted_paths = json_to_key_value_string(pathsPrograms);
    cout << "Formatted paths: " << formatted_paths << endl;
    return;
    
    // Формируем тело запроса с преобразованной строкой
    json request_body = {
        {"paths", formatted_paths}
    };
    
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer SDreD78eE6fE4"},
        {"Accept", "application/json"}
    };
    
    try {
        long http_code;
        std::string response = send_http_request(
            "POST", 
            base_url, 
            headers, 
            request_body.dump(), // Сериализуем JSON в строку
            &http_code
        );
        
        if (http_code != 200) {
            log_error("[ERROR] HTTP status code: " + std::to_string(http_code));
            return;
        }
    } catch (const std::exception& e) {
        log_error("Ошибка при выполнении запроса: " + std::string(e.what()));
    }
}

void command_processing(string text){
    const string base_url = "https://api.vubni.com/command_processing"; 
    
    json request_body = {
        {"text", text}
    };
    
    map<string, string> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer SDreD78eE6fE4"},
        {"Accept", "application/json"}
    };
    
    try {
        long http_code;
        string response = send_http_request(
            "POST", 
            base_url, 
            headers, 
            request_body.dump(),
            &http_code
        );
        if (http_code != 200) {
            log_error("[ERROR] HTTP status code: " + http_code);
            return;
        }
        
        json result = json::parse(response);
        string answer = result["answer"];
        // Нужно добавить логику синтеза речи
        cout << answer << endl;

        execute_action(result["action"]);
    } catch (const exception& e) {
        log_error("Ошибка при выполнении запроса: " + (string)e.what());
    }
}

void command_execution(string text){
    log_info("Command execution started with text: " + text);
    command_processing(text);
}