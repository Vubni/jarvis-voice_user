#ifndef API_H
#define API_H

#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <mutex>

using json = nlohmann::json;

const json& get_settings();

void load_settings(const std::string& file_path = "settings.json");
void update_settings(const json& new_settings);
void save_settings(const std::string& file_path = "settings.json");

#endif