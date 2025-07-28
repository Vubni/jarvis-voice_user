#pragma once
#include <string>

namespace AutoStart {
    bool AddToStartup(const std::wstring& appName, const std::wstring& appPath);
    bool RemoveFromStartup(const std::wstring& appName);
}