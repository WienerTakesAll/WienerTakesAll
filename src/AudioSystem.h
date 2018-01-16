#pragma once

#include <unordered_map>
#include <string>

#include "SDL_mixer.h"

class AudioSystem {

    bool init_successful = false;
    std::unordered_map<std::string, Mix_Chunk*> sound_assets;
    std::unordered_map<std::string, Mix_Music*> music_assets;

    void LoadAudioAssets();

public:
    AudioSystem();
    void PlaySound(const char* sound, const int loops = 0) const;
    void PlayMusic(const char* music, const bool force = false) const;
    void PauseMusic() const;
    void ResumeMusic() const;
    void quit();
};