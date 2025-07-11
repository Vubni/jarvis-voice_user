#include "http_client.h"

// Callback-функция для получения данных ответа
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* s)
{
    size_t newLength = size * nmemb;
    try {
        char* data = static_cast<char*>(contents);
        s->append(data, newLength);
    }
    catch (...) {
        return 0; // Ошибка — останавливаем приём
    }
    return newLength;
}

string send_http_request(
    const string& method,
    const string& url,
    const map<string, string>& headers,
    const string& post_data,
    long* out_http_code)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw runtime_error("Failed to initialize CURL");
    }

    string response;
    struct curl_slist* header_list = nullptr;

    for (const auto& [key, value] : headers) {
        string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

    // Настройка метода и тела запроса
    if (method == "GET") {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
    else if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
    }
    else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
    }
    else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        if (!post_data.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
        }
    }
    else {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        if (!post_data.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_data.size());
        }
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    if (header_list) curl_slist_free_all(header_list);

    if (out_http_code) {
        *out_http_code = http_code;
    }

    if (res != CURLE_OK) {
        ostringstream err;
        err << "CURL error: " << curl_easy_strerror(res);
        throw runtime_error(err.str());
    }

    return response;
}