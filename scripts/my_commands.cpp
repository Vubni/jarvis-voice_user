#define _HAS_STD_BYTE 0

#include "my_commands.h"
#include <Windows.h>
#include <ShlObj.h>
#include <string>
#include "logger.h"
#include "command_processor.h"
#include "keyboard.h"
#include "audioPlayer.h"

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

namespace {

bool RunFileInternal(const std::string& filePath, const char* parameters) {
    // Конвертация пути в UTF-16
    int pathLen = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, nullptr, 0);
    if (pathLen <= 0) {
        log_error("Path conversion error (MultiByteToWideChar): " + GetLastError());
        return false;
    }
    
    std::wstring wFilePath(pathLen, 0);
    if (MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, wFilePath.data(), pathLen) == 0) {
        log_error("Path conversion error 2: " + GetLastError());
        return false;
    }

    // Конвертация параметров (если есть)
    std::wstring wParameters;
    const wchar_t* paramsPtr = nullptr;
    if (parameters && *parameters != '\0') {
        int paramLen = MultiByteToWideChar(CP_UTF8, 0, parameters, -1, nullptr, 0);
        if (paramLen <= 0) {
            log_error("Parameters conversion error: " + GetLastError());
            return false;
        }
        
        wParameters.resize(paramLen);
        if (MultiByteToWideChar(CP_UTF8, 0, parameters, -1, wParameters.data(), paramLen) == 0) {
            log_error("Parameters conversion error 2: " + GetLastError());
            return false;
        }
        paramsPtr = wParameters.c_str();
    }

    // Настройка параметров запуска
    SHELLEXECUTEINFO execInfo = { sizeof(execInfo) };
    execInfo.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
    execInfo.lpVerb = L"open";
    execInfo.lpFile = wFilePath.c_str();
    execInfo.lpParameters = paramsPtr;
    execInfo.nShow = SW_SHOWNORMAL;

    // Запуск с проверкой результата
    if (!ShellExecuteEx(&execInfo)) {
        DWORD err = GetLastError();
        std::ostringstream oss;
        oss << "ShellExecuteEx failed (Error: 0x" << std::hex << err << ")";
        log_error(oss.str());
        
        // Расшифровка стандартных ошибок
        if (err == SE_ERR_NOASSOC) {
            log_error("No associated program for file type");
        } else if (err == ERROR_FILE_NOT_FOUND) {
            log_error("File not found: ");
        }
        return false;
    }
    
    return true;
}


} // namespace

// Перегрузка с параметрами
void runFile(const std::string& filePath, const std::string& parameters = "") {
    cout << "Launching: " << filePath << endl;
    
    bool success;
    if (!parameters.empty()) {
        success = RunFileInternal(filePath, parameters.c_str());
    } else {
        success = RunFileInternal(filePath, nullptr);
    }

    if (!success) {
        log_error("Failed to launch file (E0000)");
    }
}


void execute_action(string script){
    cout << script << endl;
    engine.Execute(script);
}

void mainCommands(){
    engine.RegisterFunction("runFile", &runFile, {(string)""});
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