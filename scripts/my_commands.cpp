#include "my_commands.h"

ScriptEngine engine;

void openWebsite(const string& url, bool newWindow) {
    #ifdef _WIN32
        wstring wideUrl(url.begin(), url.end());
        if (newWindow) {
            // Попытка открыть в новом окне через Chrome
            ShellExecuteW(NULL, L"open", L"chrome.exe", (L"--new-window " + wideUrl).c_str(), NULL, SW_SHOWNORMAL);
        } else {
            // Открыть в браузере по умолчанию
            ShellExecuteW(NULL, L"open", wideUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    #elif __linux__
        string command;
        if (newWindow) {
            command = "firefox -new-window '" + url + "'";
        } else {
            command = "xdg-open '" + url + "'";
        }
        system(command.c_str());
    #elif __APPLE__
        string command = "open ";
        if (newWindow) {
            command += "-n ";
        }
        command += "'" + url + "'";
        system(command.c_str());
    #else
        cerr << "Unsupported platform." << endl;
    #endif
}

void sleep(int number=1){
    Sleep(number*1000);
}

void NewWorkTable(){
    holdTwoAndPressOne("win", "ctrl", "d");
    Sleep(100);
}

void LastWorkTable(){
    holdTwoAndPressOne("win", "ctrl", "left");
    Sleep(100);
}

void NextWorkTable(){
    holdTwoAndPressOne("win", "ctrl", "right");
    Sleep(100);
}

void playAudioGreat(){
    playAudio("success/" + (string)randomizer({"1", "2"}));
}


void execute_action(string script){
    cout << script << endl;
    engine.Execute(script);
}

void mainCommands(){
    engine.RegisterFunction("playAudio", &playAudio);
    engine.RegisterFunction("playAudioGreat", &playAudioGreat);
    engine.RegisterFunction("openWebsite", &openWebsite, {false});
    engine.RegisterFunction("sleep", &sleep, {1});
    engine.RegisterFunction("pressKey", &pressKey);
    engine.RegisterFunction("holdKey", &holdKey);
    engine.RegisterFunction("releaseKey", &releaseKey);
    engine.RegisterFunction("holdAndPress", &holdAndPress);
    engine.RegisterFunction("holdTwoAndPressOne", &holdTwoAndPressOne);
    engine.RegisterFunction("NewWorkTable", &NewWorkTable);
    engine.RegisterFunction("LastWorkTable", &LastWorkTable);
    engine.RegisterFunction("NextWorkTable", &NextWorkTable);
}