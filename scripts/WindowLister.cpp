#include "WindowLister.h"
#include <vector>
#include <cstring>
#include <algorithm> // Добавлен для std::find

namespace {
    BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
        auto& windows = *reinterpret_cast<std::vector<HWND>*>(lParam);
        
        if (!IsWindowVisible(hwnd)) 
            return TRUE;

        if (GetWindowTextLengthA(hwnd) == 0) 
            return TRUE;

        char class_name[256];
        GetClassNameA(hwnd, class_name, sizeof(class_name));
        if (strcmp(class_name, "Progman") == 0 || 
            strcmp(class_name, "Button") == 0 || 
            strcmp(class_name, "Shell_TrayWnd") == 0)
            return TRUE;

        if (strstr(class_name, "Windows.UI.Core.CoreWindow"))
            return TRUE;

        windows.push_back(hwnd);
        return TRUE;
    }
}

std::vector<HWND> GetActiveWindows() {
    std::vector<HWND> windows;
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&windows));
    
    // Получаем текущее активное окно
    HWND foreground = GetForegroundWindow();
    
    // Проверяем наличие активного окна в списке
    auto it = std::find(windows.begin(), windows.end(), foreground);
    if (it != windows.end() && it != windows.begin()) {
        windows.erase(it);              // Удаляем из текущей позиции
        windows.insert(windows.begin(), foreground); // Вставляем в начало
    }
    
    return windows;
}