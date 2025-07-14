#include "installed_programs.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <objbase.h>
#include <vector>
#include <codecvt>
#include <locale>
#include <nlohmann/json.hpp>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

namespace {

std::string WStringToUTF8(const wchar_t* wstr) {
    if (!wstr) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed == 0) return "";
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, nullptr, nullptr);
    str.resize(size_needed - 1);
    return str;
}

std::wstring GetCommonStartMenuPath() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_CommonPrograms, 0, NULL, &path))) {
        std::wstring result(path);
        CoTaskMemFree(path);
        return result;
    }
    return L"";
}

std::wstring GetUserStartMenuPath() {
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, NULL, &path))) {
        std::wstring result(path);
        CoTaskMemFree(path);
        return result;
    }
    return L"";
}

void ProcessLnkFile(const std::wstring& basePath, const std::wstring& lnkPath, nlohmann::json& result, bool filter) {
    IShellLinkW* psl = nullptr;
    IPersistFile* ppf = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void**)&psl);
    if (FAILED(hr)) return;

    hr = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(hr)) {
        psl->Release();
        return;
    }

    hr = ppf->Load(lnkPath.c_str(), STGM_READ);
    if (SUCCEEDED(hr)) {
        wchar_t targetPath[MAX_PATH] = {0};
        wchar_t arguments[MAX_PATH] = {0};
        
        if (SUCCEEDED(psl->GetPath(targetPath, MAX_PATH, NULL, SLGP_RAWPATH))) {
            psl->GetArguments(arguments, MAX_PATH);
            
            // Фильтрация цели для .lnk (только если включен фильтр)
            if (filter) {
                std::wstring targetExt = PathFindExtensionW(targetPath);
                if (targetExt.empty() || _wcsicmp(targetExt.c_str(), L".exe") != 0) {
                    ppf->Release();
                    psl->Release();
                    return;
                }
            }
            
            std::wstring fullCommand = targetPath;
            if (wcslen(arguments) > 0) {
                fullCommand += L" ";
                fullCommand += arguments;
            }
            
            std::wstring relPath = lnkPath.substr(basePath.length() + 1);
            size_t dotPos = relPath.find_last_of(L'.');
            if (dotPos != std::wstring::npos && _wcsicmp(relPath.substr(dotPos).c_str(), L".lnk") == 0) {
                relPath = relPath.substr(0, dotPos);
            }

            std::string utf8Name = WStringToUTF8(relPath.c_str());
            std::string utf8Command = WStringToUTF8(fullCommand.c_str());
            
            DWORD attrs = GetFileAttributesW(targetPath);
            if (attrs != INVALID_FILE_ATTRIBUTES) {
                result[utf8Name] = utf8Command;
            }
        }
    }
    ppf->Release();
    psl->Release();
}

void ScanDirectory(const std::wstring& basePath, const std::wstring& currentPath, nlohmann::json& result, bool filter) {
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = currentPath + L"\\*";

    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) 
            continue;

        std::wstring fullPath = currentPath + L"\\" + findData.cFileName;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ScanDirectory(basePath, fullPath, result, filter);
        } else {
            bool isExe = PathMatchSpecW(findData.cFileName, L"*.exe");
            bool isLnk = PathMatchSpecW(findData.cFileName, L"*.lnk");
            
            if (filter && !isExe && !isLnk) {
                continue; // Пропускаем не .exe/.lnk при фильтрации
            }

            if (isExe) {
                std::wstring relPath = fullPath.substr(basePath.length() + 1);
                std::string utf8Name = WStringToUTF8(relPath.c_str());
                std::string utf8Path = WStringToUTF8(fullPath.c_str());
                result[utf8Name] = utf8Path;
            } else if (isLnk) {
                ProcessLnkFile(basePath, fullPath, result, filter);
            } else if (!filter) {
                // Добавляем все остальные файлы при отключенной фильтрации
                std::wstring relPath = fullPath.substr(basePath.length() + 1);
                std::string utf8Name = WStringToUTF8(relPath.c_str());
                std::string utf8Path = WStringToUTF8(fullPath.c_str());
                result[utf8Name] = utf8Path;
            }
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

} // namespace

nlohmann::json InstalledPrograms::GetInstalledPrograms(bool filter) {
    nlohmann::json result = nlohmann::json::object();

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return result;

    std::vector<std::wstring> startMenuPaths = {
        GetCommonStartMenuPath(),
        GetUserStartMenuPath()
    };

    for (const auto& path : startMenuPaths) {
        if (!path.empty()) {
            ScanDirectory(path, path, result, filter);
        }
    }

    CoUninitialize();
    return result;
}