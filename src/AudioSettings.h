#pragma once

namespace AudioSettings {
    // Sound sampling frequency in Hz. Change to "22050" if too slow.
    int MIX_FREQ_HZ = 44100;
    // Number of sound channels for output. 1 = Mono, 2 = Stereo. Indepent from mixing channels.
    int MIX_NUM_CHANNELS = 2;
    // Bytes used per output sample
    int MIX_CHUNK_SIZE = 4096;
    // Information for loading sound assets. First value is the key, second is the path.
    std::vector<std::pair<const SoundAsset, const char*>> SOUND_ASSETS_INFO = {
        {SoundAsset::BEAT, "assets/audio/beat.wav"}
    };
    // Information for loading music assets. First value is the key, second is the path.
    std::vector<std::pair<const MusicAsset, const char*>> MUSIC_ASSETS_INFO = {
        {MusicAsset::BEAT, "assets/audio/beat.wav"}
    };
}
