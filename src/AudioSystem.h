#pragma once

#include <unordered_map>
#include <string>

#include "EventSystem.h"
#include "SDL_mixer.h"

class AudioSystem : public EventSystem<AudioSystem> {
public:
    AudioSystem();
    void play_sound(const char* sound, const int loops = 0) const;
    void play_music(const char* music, const bool force = false) const;
    void pause_music() const;
    void resume_music() const;
    void quit();

private:
    bool init_successful_;
    std::unordered_map<std::string, Mix_Chunk*> sound_assets_;
    std::unordered_map<std::string, Mix_Music*> music_assets_;

    void load_audio_assets();
    void handle_keypress_event(const Event& e);
};
