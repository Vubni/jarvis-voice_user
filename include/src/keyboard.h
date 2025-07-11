#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <string>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include "core.h"

using namespace std;

void pressKey(const string& key);
void holdKey(const string& key);
void releaseKey(const string& key);
void holdAndPress(const string& key1, const string& key2);
void holdTwoAndPressOne(const string& key1, const string& key2, const string& key3);

#endif // KEYBOARD_H