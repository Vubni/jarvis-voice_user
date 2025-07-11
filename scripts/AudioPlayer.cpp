#include "audioPlayer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>

void playAudio(const std::string& filePath) {
    // Формируем полный путь к файлу
    std::string fullPath = "./voice/default/" + filePath + ".wav";
    // Инициализация SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Инициализация SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! Mix Error: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Загрузка аудиофайла
    Mix_Chunk* chunk = Mix_LoadWAV(fullPath.c_str());
    if (chunk == nullptr) {
        std::cerr << "Failed to load audio file! Mix Error: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return;
    }

    // Воспроизведение звука
    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel == -1) {
        std::cerr << "Failed to play audio! Mix Error: " << Mix_GetError() << std::endl;
        Mix_FreeChunk(chunk);
        Mix_CloseAudio();
        SDL_Quit();
        return;
    }

    // Ожидание окончания воспроизведения
    while (Mix_Playing(channel) != 0) {
        SDL_Delay(100);
    }

    // Очистка ресурсов
    Mix_FreeChunk(chunk);
    Mix_CloseAudio();
    SDL_Quit();

    return;
}