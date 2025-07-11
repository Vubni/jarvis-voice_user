#include "keyboard.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN             // Исключаем редко используемые компоненты Windows
#define _WIN32_WINNT 0x0A00              // Указываем версию Windows 10
#include <windows.h>
#elif __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif


#ifdef _WIN32
UINT keyStringToVK(const std::string& key) {
    static const std::unordered_map<std::string, UINT> keyMap = {
        {"backspace", VK_BACK}, {"tab", VK_TAB}, {"enter", VK_RETURN}, {"shift", VK_SHIFT},
        {"ctrl", VK_CONTROL}, {"alt", VK_MENU}, {"pause", VK_PAUSE}, {"capslock", VK_CAPITAL},
        {"esc", VK_ESCAPE}, {"space", VK_SPACE}, {"pageup", VK_PRIOR}, {"pagedown", VK_NEXT},
        {"end", VK_END}, {"home", VK_HOME}, {"left", VK_LEFT}, {"up", VK_UP}, {"right", VK_RIGHT},
        {"down", VK_DOWN}, {"printscreen", VK_SNAPSHOT}, {"insert", VK_INSERT}, {"delete", VK_DELETE},
        {"lwin", VK_LWIN}, {"win", VK_LWIN}, {"rwin", VK_RWIN}, {"numlock", VK_NUMLOCK}, {"scrolllock", VK_SCROLL},
        {"f1", VK_F1}, {"f2", VK_F2}, {"f3", VK_F3}, {"f4", VK_F4}, {"f5", VK_F5}, {"f6", VK_F6},
        {"f7", VK_F7}, {"f8", VK_F8}, {"f9", VK_F9}, {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12}
    };

    std::string lowerKey = toLower(key);

    auto it = keyMap.find(lowerKey);
    if (it != keyMap.end()) {
        return it->second;
    }

    // Загружаем английскую раскладку (US) один раз
    static HKL enLayout = []() {
        return LoadKeyboardLayoutW(L"00000409", KLF_NOTELLSHELL | KLF_REPLACELANG);
    }();

    if (key.length() == 1) {
        if (enLayout == nullptr) {
            // Резервный вариант: попробуем использовать текущую раскладку
            HKL currentLayout = GetKeyboardLayout(0);
            int vkCode = VkKeyScanExA(key[0], currentLayout);
            if (vkCode == -1) return 0;
            return vkCode & 0xFF;
        }

        int vkCode = VkKeyScanExA(key[0], enLayout);
        if (vkCode == -1) return 0;
        return vkCode & 0xFF;
    }

    return 0;
}

void pressKey(const string& key) {
    UINT vk = keyStringToVK(key);
    if (vk == 0) return;

    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = vk;
    SendInput(1, &ip, sizeof(INPUT));

    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void holdKey(const string& key) {
    UINT vk = keyStringToVK(key);
    if (vk == 0) return;

    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = vk;
    SendInput(1, &ip, sizeof(INPUT));
}

void releaseKey(const string& key) {
    UINT vk = keyStringToVK(key);
    if (vk == 0) return;

    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = vk;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void holdAndPress(const string& key1, const string& key2) {
    UINT vk1 = keyStringToVK(key1);
    UINT vk2 = keyStringToVK(key2);
    if (vk1 == 0 || vk2 == 0) return;

    INPUT ip = {0};
    ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = vk1;
    SendInput(1, &ip, sizeof(INPUT)); // Удержание первой клавиши

    ip.ki.wVk = vk2;
    SendInput(1, &ip, sizeof(INPUT)); // Нажатие второй клавиши
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT)); // Отпускание второй клавиши
}

void holdTwoAndPressOne(const string& key1, const string& key2, const string& key3) {
    UINT vk1 = keyStringToVK(key1);
    UINT vk2 = keyStringToVK(key2);
    UINT vk3 = keyStringToVK(key3);

    if (vk1 == 0 || vk2 == 0 || vk3 == 0) return;

    INPUT ips[3] = {0};
    for (int i = 0; i < 3; ++i) {
        ips[i].type = INPUT_KEYBOARD;
    }

    ips[0].ki.wVk = vk1;
    ips[1].ki.wVk = vk2;
    ips[2].ki.wVk = vk3;

    // Нажатие клавиш
    if (SendInput(3, ips, sizeof(INPUT)) != 3) {
        cout << "Ошибка нажатия клавиш: " << GetLastError() << endl;
        return;
    }
    Sleep(50); // Задержка для стабильности

    // Отпускание клавиш
    for (int i = 0; i < 3; ++i) {
        ips[i].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    if (SendInput(3, ips, sizeof(INPUT)) != 3) {
        cout << "Ошибка отпускания клавиш: " << GetLastError() << endl;
    }
}
#endif

// Linux реализация
#ifdef __linux__
Display* getDisplay() {
    static Display* display = XOpenDisplay(nullptr);
    return display;
}

KeyCode getKeyCodeLinux(const string& key, Display* display) {
    static const unordered_map<string, const char*> keyMap = {
        {"backspace", "BackSpace"}, {"tab", "Tab"}, {"enter", "Return"}, {"shift", "Shift_L"},
        {"ctrl", "Control_L"}, {"alt", "Alt_L"}, {"pause", "Pause"}, {"capslock", "Caps_Lock"},
        {"esc", "Escape"}, {"space", "space"}, {"pageup", "Prior"}, {"pagedown", "Next"},
        {"end", "End"}, {"home", "Home"}, {"left", "Left"}, {"up", "Up"}, {"right", "Right"},
        {"down", "Down"}, {"printscreen", "Print"}, {"insert", "Insert"}, {"delete", "Delete"},
        {"lwin", "Super_L"}, {"rwin", "Super_R"}, {"numlock", "Num_Lock"}, {"scrolllock", "Scroll_Lock"},
        {"f1", "F1"}, {"f2", "F2"}, {"f3", "F3"}, {"f4", "F4"}, {"f5", "F5"}, {"f6", "F6"},
        {"f7", "F7"}, {"f8", "F8"}, {"f9", "F9"}, {"f10", "F10"}, {"f11", "F11"}, {"f12", "F12"}
    };

    string lowerKey = key;
    transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

    auto it = keyMap.find(lowerKey);
    if (it != keyMap.end()) {
        return XKeysymToKeycode(display, XStringToKeysym(it->second));
    }

    if (key.length() == 1) {
        return XKeysymToKeycode(display, XStringToKeysym(string(1, key[0]).c_str()));
    }

    if (key.size() > 1 && key[0] == 'F') {
        int num = stoi(key.substr(1));
        if (num >= 1 && num <= 12) {
            return XKeysymToKeycode(display, XStringToKeysym(("F" + to_string(num)).c_str()));
        }
    }

    return 0;
}

void pressKey(const string& key) {
    Display* display = getDisplay();
    if (!display) return;

    KeyCode kc = getKeyCodeLinux(key, display);
    if (kc == 0) return;

    XTestFakeKeyEvent(display, kc, True, 0);
    XTestFakeKeyEvent(display, kc, False, 0);
    XFlush(display);
}

void holdKey(const string& key) {
    Display* display = getDisplay();
    if (!display) return;

    KeyCode kc = getKeyCodeLinux(key, display);
    if (kc == 0) return;

    XTestFakeKeyEvent(display, kc, True, 0);
    XFlush(display);
}

void releaseKey(const string& key) {
    Display* display = getDisplay();
    if (!display) return;

    KeyCode kc = getKeyCodeLinux(key, display);
    if (kc == 0) return;

    XTestFakeKeyEvent(display, kc, False, 0);
    XFlush(display);
}

void holdAndPress(const string& key1, const string& key2) {
    Display* display = getDisplay();
    if (!display) return;

    KeyCode kc1 = getKeyCodeLinux(key1, display);
    KeyCode kc2 = getKeyCodeLinux(key2, display);
    if (kc1 == 0 || kc2 == 0) return;

    XTestFakeKeyEvent(display, kc1, True, 0);  // Удержание первой клавиши
    XTestFakeKeyEvent(display, kc2, True, 0);  // Нажатие второй клавиши
    XTestFakeKeyEvent(display, kc2, False, 0); // Отпускание второй клавиши
    XFlush(display);
}

void holdTwoAndPressOne(const string& key1, const string& key2, const string& key3) {
    Display* display = getDisplay();
    if (!display) return;

    KeyCode kc1 = getKeyCodeLinux(key1, display);
    KeyCode kc2 = getKeyCodeLinux(key2, display);
    KeyCode kc3 = getKeyCodeLinux(key3, display);

    if (kc1 == 0 || kc2 == 0 || kc3 == 0) return;

    XTestFakeKeyEvent(display, kc1, True, 0);
    XTestFakeKeyEvent(display, kc2, True, 0);
    XTestFakeKeyEvent(display, kc3, True, 0);
    XTestFakeKeyEvent(display, kc3, False, 0);
    XTestFakeKeyEvent(display, kc2, False, 0);
    XTestFakeKeyEvent(display, kc1, False, 0);
    XFlush(display);
}
#endif

// macOS реализация
#ifdef __APPLE__
CGKeyCode getKeyCodeMac(const string& key) {
    static const unordered_map<string, CGKeyCode> keyMap = {
        {"backspace", 51}, {"tab", 48}, {"enter", 36}, {"shift", 56}, {"ctrl", 59},
        {"alt", 58}, {"pause", 110}, {"capslock", 57}, {"esc", 53}, {"space", 49},
        {"pageup", 116}, {"pagedown", 121}, {"end", 119}, {"home", 115}, {"left", 123},
        {"up", 126}, {"right", 124}, {"down", 125}, {"printscreen", 105}, {"insert", 114},
        {"delete", 117}, {"lwin", 55}, {"rwin", 55}, {"numlock", 71}, {"scrolllock", 107},
        {"f1", 122}, {"f2", 120}, {"f3", 99}, {"f4", 118}, {"f5", 96}, {"f6", 97},
        {"f7", 98}, {"f8", 100}, {"f9", 101}, {"f10", 109}, {"f11", 103}, {"f12", 111}
    };

    string lowerKey = key;
    transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

    auto it = keyMap.find(lowerKey);
    if (it != keyMap.end()) {
        return it->second;
    }

    if (key.length() == 1) {
        // Пример простого маппинга для букв
        char c = tolower(key[0]);
        if (c >= 'a' && c <= 'z') {
            return static_cast<CGKeyCode>(4 + (c - 'a'));
        }
    }

    if (key.size() > 1 && key[0] == 'F') {
        int num = stoi(key.substr(1));
        if (num >= 1 && num <= 12) {
            return static_cast<CGKeyCode>(122 + (num - 1));
        }
    }

    return 0;
}

void pressKey(const string& key) {
    CGKeyCode kc = getKeyCodeMac(key);
    if (kc == 0) return;

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSession);
    CGEventRef downEvent = CGEventCreateKeyboardEvent(source, kc, true);
    CGEventRef upEvent = CGEventCreateKeyboardEvent(source, kc, false);

    CGEventSetType(downEvent, kCGEventKeyDown);
    CGEventSetType(upEvent, kCGEventKeyUp);

    CGEventPost(kCGHIDEventTap, downEvent);
    CGEventPost(kCGHIDEventTap, upEvent);

    CFRelease(upEvent);
    CFRelease(downEvent);
    CFRelease(source);
}

void holdKey(const string& key) {
    CGKeyCode kc = getKeyCodeMac(key);
    if (kc == 0) return;

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSession);
    CGEventRef event = CGEventCreateKeyboardEvent(source, kc, true);
    CGEventSetType(event, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    CFRelease(source);
}

void releaseKey(const string& key) {
    CGKeyCode kc = getKeyCodeMac(key);
    if (kc == 0) return;

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSession);
    CGEventRef event = CGEventCreateKeyboardEvent(source, kc, false);
    CGEventSetType(event, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    CFRelease(source);
}

void holdAndPress(const string& key1, const string& key2) {
    CGKeyCode kc1 = getKeyCodeMac(key1);
    CGKeyCode kc2 = getKeyCodeMac(key2);
    if (kc1 == 0 || kc2 == 0) return;

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSession);

    // Удержание первой клавиши
    CGEventRef down1 = CGEventCreateKeyboardEvent(source, kc1, true);
    CGEventSetType(down1, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, down1);

    // Нажатие и отпускание второй клавиши
    CGEventRef down2 = CGEventCreateKeyboardEvent(source, kc2, true);
    CGEventSetType(down2, kCGEventKeyDown);
    CGEventPost(kCGHIDEventTap, down2);

    CGEventRef up2 = CGEventCreateKeyboardEvent(source, kc2, false);
    CGEventSetType(up2, kCGEventKeyUp);
    CGEventPost(kCGHIDEventTap, up2);

    // Освобождение ресурсов
    CFRelease(down1);
    CFRelease(down2);
    CFRelease(up2);
    CFRelease(source);
}

void holdTwoAndPressOne(const string& key1, const string& key2, const string& key3) {
    CGKeyCode kc1 = getKeyCodeMac(key1);
    CGKeyCode kc2 = getKeyCodeMac(key2);
    CGKeyCode kc3 = getKeyCodeMac(key3);

    if (kc1 == 0 || kc2 == 0 || kc3 == 0) return;

    CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateCombinedSession);

    CGEventRef down1 = CGEventCreateKeyboardEvent(source, kc1, true);
    CGEventRef down2 = CGEventCreateKeyboardEvent(source, kc2, true);
    CGEventRef down3 = CGEventCreateKeyboardEvent(source, kc3, true);
    CGEventRef up3 = CGEventCreateKeyboardEvent(source, kc3, false);
    CGEventRef up2 = CGEventCreateKeyboardEvent(source, kc2, false);
    CGEventRef up1 = CGEventCreateKeyboardEvent(source, kc1, false);

    CGEventSetType(down1, kCGEventKeyDown);
    CGEventSetType(down2, kCGEventKeyDown);
    CGEventSetType(down3, kCGEventKeyDown);
    CGEventSetType(up3, kCGEventKeyUp);
    CGEventSetType(up2, kCGEventKeyUp);
    CGEventSetType(up1, kCGEventKeyUp);

    CGEventPost(kCGHIDEventTap, down1);
    CGEventPost(kCGHIDEventTap, down2);
    CGEventPost(kCGHIDEventTap, down3);
    CGEventPost(kCGHIDEventTap, up3);
    CGEventPost(kCGHIDEventTap, up2);
    CGEventPost(kCGHIDEventTap, up1);

    CFRelease(down1);
    CFRelease(down2);
    CFRelease(down3);
    CFRelease(up3);
    CFRelease(up2);
    CFRelease(up1);
    CFRelease(source);
}
#endif