#include "api.h"
#include <vector>

bool start = false;

void command_execution(string text){
    if (!start){
        start = true;
        log_info("Create session");
        // create_session(pathsPrograms);
    }
    log_info("Command execution started with text: " + text);
    command_processing(text);
}

void create_session(json pathsPrograms){
    const string base_url = "https://api.vubni.com/create_session"; 
    
    json request_body = {
        {"paths", pathsPrograms}
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
    } catch (const exception& e) {
        log_error("Ошибка при выполнении запроса: " + (string)e.what());
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