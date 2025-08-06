#include "api.h"
#include <vector>
#include "CustomNotification.h"
#include "mainwindow.h"
#include "settings.h"

bool create_session(json pathsPrograms) {
    const std::string base_url = "https://api.vubni.com/create_session"; 
    
    // Формируем тело запроса с преобразованной строкой
    json request_body = {
        {"paths", pathsPrograms}
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
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        log_error("Ошибка при выполнении запроса: " + std::string(e.what()));
        return false;
    }
}

bool command_processing(const string text_ru, string text_en){
    json settings = get_settings();
    const string base_url = "https://api.vubni.com/command_processing"; 

    if (!settings["speech_en"]) text_en = "";
    
    json request_body = {
        {"text_ru", text_ru},
        {"text_en", text_en},
        {"save_cache", settings["save_cache"]}
    };
    
    map<string, string> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + settings["token"]},
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
            return false;
        }
        
        json result = json::parse(response);

        if (!result.is_object()) {
            log_error("Ожидался JSON-объект, получен: " + result.dump());
            return false;
        }

        if (result["answer"] == "" && result["action"] == "") return false;

        if (result["answer"] != "") showCustomNotification("Джарвис", result["answer"]);
        if (result["action"] != "") execute_action(result["action"]);

        append_user_text_console(text_ru);
        if (result["answer"] != "")
            append_jarvis_text_console(result["answer"]);
        else if (result["action"] != "")
            append_jarvis_text_console("Выполняю");
        return true;
    } catch (const exception& e) {
        log_error("Ошибка при выполнении запроса: " + (string)e.what());
        return false;
    }
}

bool command_execution(const string text_ru, string text_en) {
    log_info("Command execution started with text: RU: " + text_ru + "| EN: " + text_en);
    return command_processing(text_ru, text_en);
}