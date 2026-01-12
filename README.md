# Jarvis Voice Assistant (Клиентская часть)
![C++](https://img.shields.io/badge/C++-00599C?logo=c%2b%2b&logoColor=white)
![Qt6](https://img.shields.io/badge/Qt-41CD52?logo=qt&logoColor=white)
![Vosk](https://img.shields.io/badge/Vosk-Speech%20Recognition-blue?logo=google-voice&logoColor=white)
![Windows](https://img.shields.io/badge/Windows-0078D6?logo=windows&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?logo=cmake&logoColor=white)
![SDL2](https://img.shields.io/badge/SDL2-000000?logo=steamdeck&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green)

Голосовой ассистент на C++ с интерфейсом Qt6, локальным распознаванием речи через Vosk и поддержкой автоматизации Windows.

[Серверная часть](https://github.com/Vubni/jarvis-voice_server)

## Описание

Jarvis — локальный голосовой помощник, работающий по следующему принципу:

1. Постоянно слушает микрофон через Vosk.
2. После ключевого слова «Джарвис»:
   - подсвечивает края экрана;
   - приглушает громкость всех программ на 80%;
   - начинает активный режим.
3. Отправляет распознанную фразу на сервер.
4. Получает от сервера:
   - текст ответа нейросети;
   - список команд.
5. Показывает текст через собственное Windows‑уведомление.
6. Выполняет команды (открытие приложений, закрытие окон и т.п.).

Скрипт умеет находить установленные на ПК программы и открывать их.  
Есть возможность временно отключить микрофон.  
Подразумевается расширение функциональности через дополнительные модули и голоса.

## Структура проекта
```
main.cpp                 # Главный файл
design/                  # Qt6 .ui интерфейс
scripts/                 # Основная логика
include/                 # Заголовки
libs/                    # Библиотеки
models/vosk-small/       # Модель Vosk
voice/                   # Голоса по умолчанию
styles/                  # Стили
images/                  # Иконки и изображения
logs/                    # Логи
SDL2/                    # SDL2 (если используется)
settings.json            # Настройки
CMakeLists.txt           # CMake проект
```


## Возможности

- Локальное распознавание речи (Vosk)
- Работа в фоновом режиме и в трее
- Подсветка экрана при активации
- Приглушение всех программ
- Отправка фразы на сервер и получение ответа нейросети
- Показ уведомлений
- Выполнение системных команд
- Отключение микрофона
- Поддержка расширений

## Технологии

- C++17  
- Qt6  
- Vosk  
- SDL2  
- Windows API  
- CMake

## Сборка
```bash
mkdir build
cd build
cmake ..
cmake --build .
```


## Планируемые улучшения

- Новые голоса
- Система модулей
- Обучение новым командам
- Расширение сценариев взаимодействия
