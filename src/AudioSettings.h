#pragma once

#include "MusicAsset.h"
#include "SoundAsset.h"

struct AudioSettings {
    // Sound sampling frequency in Hz. Change to "22050" if too slow.
    int mix_freq_hz = 44100;
    // Number of sound channels for output. 1 = Mono, 2 = Stereo. Indepent from mixing channels.
    int mix_num_channels = 2;
    // Bytes used per output sample
    int mix_chunk_size = 4096;
    // Information for loading sound assets. First value is the key, second is the path.
    std::vector<std::pair<const SoundAsset, const std::string>> sound_assets_info = {
        {SoundAsset::BOING, "assets/audio/boing.wav"},
        {SoundAsset::PLAYER_1_HAS_THE_WIENER, "assets/audio/player_1_has_the_wiener.wav"},
        {SoundAsset::PLAYER_2_HAS_THE_WIENER, "assets/audio/player_2_has_the_wiener.wav"},
        {SoundAsset::PLAYER_3_HAS_THE_WIENER, "assets/audio/player_3_has_the_wiener.wav"},
        {SoundAsset::PLAYER_4_HAS_THE_WIENER, "assets/audio/player_4_has_the_wiener.wav"},
        {SoundAsset::CHEERING, "assets/audio/cheering.wav"}
    };
    // Information for loading music assets. First value is the key, second is the path.
    std::vector<std::pair<const MusicAsset, const std::string>> music_assets_info = {
        {MusicAsset::START_MENU, "assets/audio/economical-shoppers.wav"},
        {MusicAsset::IN_GAME, "assets/audio/twister.wav"}
    };
};
