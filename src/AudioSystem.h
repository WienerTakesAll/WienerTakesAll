#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "EventSystem.h"
#include "MusicAsset.h"
#include "SoundAsset.h"

#include "SDL_mixer.h"

struct AudioSettings;

class AudioSystem : public EventSystem<AudioSystem> {

public:
    AudioSystem(const AudioSettings& settings);
    bool init();
    void play_sound(const SoundAsset sound_type, const int loops = 0) const;
    void play_music(const MusicAsset music_type, const bool force = false) const;
    void pause_music() const;
    void resume_music() const;
    void quit();

private:
    bool init_successful_;

    std::unordered_map<int, Mix_Chunk*> sound_assets_;
    std::unordered_map<int, Mix_Music*> music_assets_;
    const AudioSettings& settings_;

    bool load_audio_assets();
    void handle_update_settings_event(const Event& e);
    void handle_vehicle_collision_event(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_new_it(const Event& e);
};
