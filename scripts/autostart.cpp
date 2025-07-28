#include "autostart.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <comutil.h>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")

namespace {

bool GetStartupFolderPath(std::wstring& startupPath) {
    PWSTR path = nullptr;
    if (SHGetKnownFolderPath(FOLDERID_Startup, 0, nullptr, &path) != S_OK) {
        return false;
    }
    startupPath = path;
    CoTaskMemFree(path);
    return true;
}

} // namespace

namespace AutoStart {

bool AddToStartup(const std::wstring& appName, const std::wstring& appPath) {
    std::wstring startupDir;
    if (!GetStartupFolderPath(startupDir)) {
        return false;
    }

    // Создание пути к ярлыку
    std::wstring shortcutPath = startupDir + L"\\" + appName + L".lnk";

    // Инициализация COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;

    IShellLink* pShellLink = nullptr;
    IPersistFile* pPersistFile = nullptr;

    do {
        // Создание объекта IShellLink
        hr = CoCreateInstance(
            CLSID_ShellLink, 
            nullptr, 
            CLSCTX_INPROC_SERVER, 
            IID_IShellLink, 
            (void**)&pShellLink
        );
        if (FAILED(hr)) break;

        // Установка пути к приложению
        hr = pShellLink->SetPath(appPath.c_str());
        if (FAILED(hr)) break;

        // Установка рабочей директории
        std::wstring::size_type pos = appPath.find_last_of(L"\\/");
        std::wstring workingDir = appPath.substr(0, pos);
        hr = pShellLink->SetWorkingDirectory(workingDir.c_str());
        if (FAILED(hr)) break;

        // Сохранение ярлыка
        hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
        if (FAILED(hr)) break;

        hr = pPersistFile->Save(shortcutPath.c_str(), TRUE);
    } while (false);

    // Освобождение ресурсов
    if (pPersistFile) pPersistFile->Release();
    if (pShellLink) pShellLink->Release();
    CoUninitialize();

    return SUCCEEDED(hr);
}

bool RemoveFromStartup(const std::wstring& appName) {
    std::wstring startupDir;
    if (!GetStartupFolderPath(startupDir)) {
        return false;
    }

    std::wstring shortcutPath = startupDir + L"\\" + appName + L".lnk";
    return DeleteFileW(shortcutPath.c_str()) || GetLastError() == ERROR_FILE_NOT_FOUND;
}

} // namespace AutoStart