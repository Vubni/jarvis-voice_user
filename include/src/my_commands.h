#ifndef MY_COMMANDS_H
#define MY_COMMANDS_H

#include <iostream>
#include <windows.h>
#include <string>
#include "logger.h"
#include "command_processor.h"
#include "keyboard.h"
#include "audioPlayer.h"

using namespace std;

void execute_action(string text);
void mainCommands();

#endif