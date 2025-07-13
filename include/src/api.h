#ifndef API_H
#define API_H

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "http_client.h"
#include "logger.h"
#include "core.h"
#include "my_commands.h"
#include "installed_programs.h"

using namespace std;
using json = nlohmann::json;

void command_execution(string text);

#endif // COMMAND_PROCESSOR_H