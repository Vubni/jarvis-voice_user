#include "authorization.h"
#include "http_client.h"
#include "logger.h"
#include "nlohmann/json.hpp"
#include "settings.h"
using namespace std;

bool check_token(string token){
    const string base_url = "https://api.vubni.com/profile/check_token";
    map<string, string> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + token},
        {"Accept", "application/json"}
    };
    try {
        long http_code;
        string response = send_http_request(
            "POST", 
            base_url, 
            headers, 
            "",
            &http_code
        );
        if (http_code == 204) {
            return true;
        } else {
            return false;
        }
    } catch (const std::exception& e) {
        log_error("Ошибка при выполнении запроса: " + std::string(e.what()));
        return false;
    }
}

bool checker_authorization(){
    json settings = get_settings();
    if (settings["token"] == "") return false;
    return check_token(settings["token"]);
}


bool authorization(string login, string password) {
    const string base_url = "https://api.vubni.com/auth/auth";
    map<string, string> headers = {
        {"Content-Type", "application/json"},
        {"Accept", "application/json"}
    };
    json body = {
        {"identifier", login},
        {"password", password}
    };
    try {
        long http_code;
        string response = send_http_request(
            "POST", 
            base_url, 
            headers, 
            body.dump(),
            &http_code
        );
        if (http_code == 200) {
            json response_json = json::parse(response);
            json settings = get_settings();
            settings["token"] = response_json["token"].get<std::string>();
            update_settings(settings);
            save_settings();
            return true;
        } else {
            json response_json = json::parse(response);
            cout << "Authorization failed: " << response_json["error"] << endl;
            return false;
        }
    } catch (const std::exception& e) {
        log_error("Ошибка при выполнении запроса: " + std::string(e.what()));
        return false;
    }
}