#include <iostream>
#include <utility>
#include <vector>

#include "SDL.h"

#include "AudioSystem.h"
#include "AudioSettings.h"
#include "GameState.h"

AudioSystem::AudioSystem(const AudioSettings& settings)
    : init_successful_(false)
    , settings_(settings) {
}

bool AudioSystem::init() {
    if (Mix_OpenAudio(settings_.mix_freq_hz, MIX_DEFAULT_FORMAT, settings_.mix_num_channels, settings_.mix_chunk_size) != 0) {
        std::cerr << "Could not initialize SDL Mixer" << std::endl;
        init_successful_ = false;
        return false;
    }

    // Load all audio files
    if (!load_audio_assets()) {
        init_successful_ = false;
        return false;
    }

    add_event_handler(EventType::RELOAD_SETTINGS_EVENT, &AudioSystem::handle_update_settings_event, this);
    add_event_handler(EventType::VEHICLE_COLLISION, &AudioSystem::handle_vehicle_collision_event, this);
    add_event_handler(EventType::NEW_GAME_STATE, &AudioSystem::handle_new_game_state, this);
    add_event_handler(EventType::NEW_IT, &AudioSystem::handle_new_it, this);
    init_successful_ = true;
    return init_successful_;
}

bool AudioSystem::load_audio_assets() {
    // Load all sound assets
    for (auto& sound_asset_info : settings_.sound_assets_info) {
        const int key = (int) sound_asset_info.first;
        const std::string path = sound_asset_info.second;
        sound_assets_[key] = Mix_LoadWAV(path.c_str());

        std::cout << "Loading sound: " << path << std::endl;

        if (sound_assets_.at(key) == nullptr) {
            std::cerr << "Failed to load sound: " << key << std::endl;
            return false;
        }
    }

    // Load all music assets
    for (auto& music_asset_info : settings_.music_assets_info) {
        const int key = (int) music_asset_info.first;
        const std::string path = music_asset_info.second;
        music_assets_[key] = Mix_LoadMUS(path.c_str());

        std::cout << "Loading music: " << path << std::endl;

        if (music_assets_.at(key) == nullptr) {
            std::cerr << "Failed to load music: " << key << std::endl;
            return false;
        }
    }

    return true;
}

void AudioSystem::handle_update_settings_event(const Event& event) {
    // Apply any changes
}

void AudioSystem::handle_vehicle_collision_event(const Event& e) {
    play_sound(SoundAsset::BOING);
}

void AudioSystem::play_sound(const SoundAsset sound_type, const int loops /*= 0*/) const {
    if (!init_successful_) {
        return;
    }

    const int sound = (int) sound_type;

    Mix_Chunk* asset = sound_assets_.at(sound);

    if (!asset) {
        std::cerr << "Unable to play sound: " << sound << std::endl;
        return;
    }

    Mix_PlayChannel(-1, asset, loops);
    std::cout << "Audio played: " << sound << " loops: " << loops << std::endl;
}

void AudioSystem::play_music(const MusicAsset music_type, const bool force /* = false*/) const {
    if (!init_successful_) {
        return;
    }

    const int music = (int) music_type;

    Mix_Music* asset = music_assets_.at(music);

    if (Mix_PlayingMusic() == 1) {
        if (force) {
            Mix_HaltMusic();
            Mix_PlayMusic(asset, -1);
        }
    } else {
        Mix_PlayMusic(asset, -1);
    }

    std::cout << "Music played: " << music << ", forced: " << force << std::endl;
}

void AudioSystem::pause_music() const {
    if (!init_successful_) {
        return;
    }

    if (Mix_PlayingMusic() == 1) {
        Mix_PauseMusic();
    }
}

void AudioSystem::resume_music() const {
    if (!init_successful_) {
        return;
    }

    if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
    }
}

void AudioSystem::quit() {
    if (!init_successful_) {
        return;
    }

    // Free sound assets
    for (auto it = sound_assets_.begin(); it != sound_assets_.end(); ++it) {
        Mix_FreeChunk(it->second);
        it->second = nullptr;
    }

    // Free music assets
    for (auto it = music_assets_.begin(); it != music_assets_.end(); ++it) {
        Mix_FreeMusic(it->second);
        it->second = nullptr;
    }

    // Clear asset sets
    sound_assets_.clear();
    music_assets_.clear();
    std::cout << "All sound assets cleared" << std::endl;

    Mix_Quit();
}

void AudioSystem::handle_new_game_state(const Event& e) {
    int new_state = e.get_value<int>("state", true).first;

    switch (new_state) {
        case GameState::START_MENU:
            play_music(MusicAsset::START_MENU, true);
            break;

        case GameState::IN_GAME:
            play_music(MusicAsset::IN_GAME, true);
            break;

        case GameState::END_GAME:
            play_sound(SoundAsset::CHEERING);
            break;
    }
}

void AudioSystem::handle_new_it(const Event& e) {
    int object_id = e.get_value<int>("object_id", true).first;

    switch (object_id) {
        case 0:
            play_sound(SoundAsset::PLAYER_1_HAS_THE_WIENER);
            break;

        case 1:
            play_sound(SoundAsset::PLAYER_2_HAS_THE_WIENER);
            break;

        case 2:
            play_sound(SoundAsset::PLAYER_3_HAS_THE_WIENER);
            break;

        case 3:
            play_sound(SoundAsset::PLAYER_4_HAS_THE_WIENER);
            break;
    }
}

