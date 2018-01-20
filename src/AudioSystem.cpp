#include <iostream>

#include "AudioSystem.h"

#include "SDL.h"
#include "SDL_mixer.h"

namespace {
    const int MIX_FREQ_HZ = 44100; // Sound sampling frequency in Hz. Change to "22050" if too slow.
    const int MIX_NUM_CHANNELS = 2; // Number of sound channels for output
    const int MIX_CHUNK_SIZE = 4096; // Bytes used per output sample
}

AudioSystem::AudioSystem()
: init_successful_(false) {
    // Initialize SDL Mixer
    init_successful_ = Mix_OpenAudio(MIX_FREQ_HZ, MIX_DEFAULT_FORMAT, MIX_NUM_CHANNELS, MIX_CHUNK_SIZE) != 1;

    if (!init_successful_) {
        std::cerr << "AudioSystem initialization failed" << std::endl;
        return;
    }

    // Load all audio files
    load_audio_assets();

    add_event_handler(EventType::KEYPRESS_EVENT, &AudioSystem::handle_keypress_event, this);
}

void AudioSystem::load_audio_assets() {
    // Load all sound assets
    sound_assets_["beat"] = Mix_LoadWAV("assets/audio/beat.wav");

    if (sound_assets_.at("beat") == nullptr) {
        std::cerr << "Failed to load sound: beat.wav" << std::endl;
    }

    // Load all music assets
    music_assets_["beat"] = Mix_LoadMUS("assets/audio/beat.wav");

    if (music_assets_.at("beat") == nullptr) {
        std::cerr << "Failed to load music: beat.wav" << std::endl;
    }
}

void AudioSystem::handle_keypress_event(const Event& e) {
    int player_id = e.get_value<int>("player_id", -1);
    int key = e.get_value<int>("key", -1);
    int value = e.get_value<int>("value", 0);

    switch (key) {
        case SDLK_LEFT:
            play_sound("beat");
            break;

        case SDLK_RIGHT:
            play_music("beat");
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

void AudioSystem::play_sound(const char* sound, const int loops /*= 0*/) const {
    if (!init_successful_) {
        return;
    }

    Mix_Chunk* asset = sound_assets_.at(sound);

    if (!asset) {
        std::cerr << "Unable to play sound: " << sound << std::endl;
        return;
    }

    Mix_PlayChannel(-1, asset, loops);
    std::cout << "Audio played: " << sound << " loops: " << loops << std::endl;
}

void AudioSystem::play_music(const char* music, const bool force /* = false*/) const {
    if (!init_successful_) {
        return;
    }

    Mix_Music* asset = music_assets_.at(music);

    if (Mix_PlayingMusic() == 1) {
        if (force) {
            Mix_HaltMusic();
            Mix_PlayMusic(asset, -1);
        }
    } else {
        Mix_PlayMusic(asset, -1);
    }
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
