#pragma once

#include <unordered_map>
#include <string>

#include "EventSystem.h"
#include "SDL_mixer.h"

class AudioSystem : public EventSystem<AudioSystem> {

    bool init_successful_ = false;
    std::unordered_map<std::string, Mix_Chunk*> sound_assets_;
    std::unordered_map<std::string, Mix_Music*> music_assets_;

    bool load_audio_assets();
    void handle_keypress_event(const Event& e);

public:
    AudioSystem();
    void play_sound(const char* sound, const int loops = 0) const;
    void play_music(const char* music, const bool force = false) const;
    void pause_music() const;
    void resume_music() const;
    void quit();
};
