#ifndef WINDOWLISTER_H
#define WINDOWLISTER_H
#pragma once

#include <vector>
#include <windows.h>

std::vector<HWND> GetActiveWindows();

#endif // LOGGER_H