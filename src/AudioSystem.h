#pragma once

#include <unordered_map>
#include <string>

#include "EventSystem.h"
#include "SDL_mixer.h"

enum class SoundType {BEAT};
enum class MusicType {BEAT};

class AudioSystem : public EventSystem<AudioSystem> {

public:
    AudioSystem();
    void play_sound(const SoundType sound_type, const int loops = 0) const;
    void play_music(const MusicType music_type, const bool force = false) const;
    void pause_music() const;
    void resume_music() const;
    void quit();

private:
    bool init_successful_ = false;
    std::unordered_map<int, Mix_Chunk*> sound_assets_;
    std::unordered_map<int, Mix_Music*> music_assets_;

    bool load_audio_assets();
    void handle_keypress_event(const Event& e);
};
