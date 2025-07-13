#include "WindowLister.h"
#include <vector>
#include <cstring>

namespace {
    BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
        auto& windows = *reinterpret_cast<std::vector<HWND>*>(lParam);
        
        // Проверка видимости окна
        if (!IsWindowVisible(hwnd)) 
            return TRUE;

        // Игнорирование окон без заголовка
        if (GetWindowTextLengthA(hwnd) == 0) 
            return TRUE;

        // Игнорирование системных окон (панель задач, рабочий стол)
        char class_name[256];
        GetClassNameA(hwnd, class_name, sizeof(class_name));
        if (strcmp(class_name, "Progman") == 0 ||   // Рабочий стол
            strcmp(class_name, "Button") == 0 ||     // Кнопки пуска (старые версии)
            strcmp(class_name, "Shell_TrayWnd") == 0) // Панель задач
            return TRUE;

        // Игнорирование UWP-приложений (не отображаются в Alt+Tab)
        if (strstr(class_name, "Windows.UI.Core.CoreWindow"))
            return TRUE;

        windows.push_back(hwnd);
        return TRUE;
    }
}

std::vector<HWND> GetActiveWindows() {
    std::vector<HWND> windows;
    
    // Собираем все подходящие окна
    EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&windows));
    
    // Сортируем окна по Z-порядку (активное окно — первое)
    std::vector<HWND> ordered;
    for (HWND hwnd = GetTopWindow(nullptr); hwnd != nullptr; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {
        if (auto it = std::find(windows.begin(), windows.end(), hwnd); it != windows.end()) {
            ordered.push_back(hwnd);
        }
    }
    
    return ordered;
}