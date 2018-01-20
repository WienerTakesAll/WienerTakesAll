#include <iostream>
#include <utility>
#include <vector>

#include "AudioSystem.h"

#include "SDL.h"
#include "SDL_mixer.h"

namespace {
    // Sound sampling frequency in Hz. Change to "22050" if too slow.
    const int MIX_FREQ_HZ = 44100;
    // Number of sound channels for output. 1 = Mono, 2 = Stereo. Indepent from mixing channels.
    const int MIX_NUM_CHANNELS = 2;
    // Bytes used per output sample
    const int MIX_CHUNK_SIZE = 4096;
    // Information for loading sound assets. First value is the key, second is the path.
    const std::vector<std::pair<const SoundType, const char*>> SOUND_ASSETS_INFO = {
        {SoundType::BEAT, "assets/audio/beat.wav"}
    };
    // Information for loading music assets. First value is the key, second is the path.
    const std::vector<std::pair<const MusicType, const char*>> MUSIC_ASSETS_INFO = {
        {MusicType::BEAT, "assets/audio/beat.wav"}
    };
}

AudioSystem::AudioSystem() {
    // Initialize SDL Mixer
    init_successful_ = Mix_OpenAudio(MIX_FREQ_HZ, MIX_DEFAULT_FORMAT, MIX_NUM_CHANNELS, MIX_CHUNK_SIZE) != 1;

    if (!init_successful_) {
        std::cerr << "AudioSystem initialization failed" << std::endl;
        return;
    }

    // Load all audio files
    init_successful_ = load_audio_assets();

    if ( !init_successful_) {
        std::cerr << "AudioSystem initialization failed" << std::endl;
        return;
    }

    add_event_handler(EventType::KEYPRESS_EVENT, &AudioSystem::handle_keypress_event, this);
}

bool AudioSystem::load_audio_assets() {
    // Load all sound assets
    for (auto& sound_asset_info : SOUND_ASSETS_INFO) {
        const int key = (int) sound_asset_info.first;
        const char* path = sound_asset_info.second;
        sound_assets_[key] = Mix_LoadWAV(path);

        if (sound_assets_.at(key) == nullptr) {
            std::cerr << "Failed to load sound: " << key << std::endl;
            return false;
        }
    }

    // Load all music assets
    for (auto& music_asset_info : MUSIC_ASSETS_INFO) {
        const int key = (int) music_asset_info.first;
        const char* path = music_asset_info.second;
        music_assets_[key] = Mix_LoadMUS(path);

        if (music_assets_.at(key) == nullptr) {
            std::cerr << "Failed to load music: " << key << std::endl;
            return false;
        }
    }

    return true;
}

void AudioSystem::handle_keypress_event(const Event& e) {
    int player_id = e.get_value<int>("player_id", -1);
    int key = e.get_value<int>("key", -1);
    int value = e.get_value<int>("value", 0);

    switch (key) {
        case SDLK_LEFT:
            play_sound(SoundType::BEAT);
            break;

        case SDLK_RIGHT:
            play_music(MusicType::BEAT);
            break;

        case SDLK_UP:
            resume_music();
            break;

        case SDLK_DOWN:
            pause_music();
            break;

        default:
            break;
    }
}

void AudioSystem::play_sound(const SoundType sound_type, const int loops /*= 0*/) const {
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

void AudioSystem::play_music(const MusicType music_type, const bool force /* = false*/) const {
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
