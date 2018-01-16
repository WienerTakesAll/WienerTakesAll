#include <iostream>

#include "AudioSystem.h"

#include "SDL.h"
#include "SDL_mixer.h"

namespace {
    const int MIX_FREQ_HZ = 44100; // Sound sampling frequency in Hz. Change to "22050" if too slow.
    const int MIX_NUM_CHANNELS = 2; // Number of sound channels for output
    const int MIX_CHUNK_SIZE = 4096; // Bytes used per output sample
}

AudioSystem::AudioSystem() {
    // Initialize SDL Mixer
    init_successful = Mix_OpenAudio(MIX_FREQ_HZ, MIX_DEFAULT_FORMAT, MIX_NUM_CHANNELS, MIX_CHUNK_SIZE) != 1;

    if (!init_successful) {
        std::cerr << "AudioSystem initialization failed" << std::endl;
        return;
    }

    // Load all audio files
    LoadAudioAssets();
}

void AudioSystem::LoadAudioAssets() {
    // Load all sound assets
    sound_assets["beat"] = Mix_LoadWAV("assets/audio/beat.wav");

    if (sound_assets.at("beat") == nullptr) {
        std::cerr << "Failed to load sound: beat.wav" << std::endl;
    }

    // Load all music assets
    music_assets["beat"] = Mix_LoadMUS("assets/audio/beat.wav");

    if (music_assets.at("beat") == nullptr) {
        std::cerr << "Failed to load music: beat.wav" << std::endl;
    }
}

void AudioSystem::PlaySound(const char* sound, const int loops /*= 0*/) const {
    if (!init_successful) {
        return;
    }

    Mix_Chunk* asset = sound_assets.at(sound);

    if (!asset) {
        std::cerr << "Unable to play sound: " << sound << std::endl;
        return;
    }

    Mix_PlayChannel(-1, asset, loops);
    std::cout << "Audio played: " << sound << " loops: " << loops << std::endl;
}

void AudioSystem::PlayMusic(const char* music, const bool force /* = false*/) const {
    if (!init_successful) {
        return;
    }

    Mix_Music* asset = music_assets.at(music);

    if (Mix_PlayingMusic() == 1) {
        if (force) {
            Mix_HaltMusic();
            Mix_PlayMusic(asset, -1);
        }
    } else {
        Mix_PlayMusic(asset, -1);
    }
}

void AudioSystem::PauseMusic() const {
    if (!init_successful) {
        return;
    }

    if (Mix_PlayingMusic() == 1) {
        Mix_PauseMusic();
    }
}

void AudioSystem::ResumeMusic() const {
    if (!init_successful) {
        return;
    }

    if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
    }
}

void AudioSystem::quit() {
    if (!init_successful) {
        return;
    }

    // Free sound assets
    for (auto it = sound_assets.begin(); it != sound_assets.end(); ++it) {
        Mix_FreeChunk(it->second);
        it->second = nullptr;
    }

    // Free music assets
    for (auto it = music_assets.begin(); it != music_assets.end(); ++it) {
        Mix_FreeMusic(it->second);
        it->second = nullptr;
    }

    // Clear asset sets
    sound_assets.clear();
    music_assets.clear();

    Mix_Quit();
}