#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <map>
#include <curl/curl.h>
#include <sstream>
#include <stdexcept>

using namespace std;

string send_http_request(
    const string& method,
    const string& url,
    const map<string, string>& headers,
    const string& post_data,
    long* out_http_code = nullptr);

#endif // HTTP_CLIENT_H