#include "SettingsSystem.h"
#include "InputSettings.h"
#include "AudioSettings.h"
#include <SDL.h>

namespace {
    typedef const std::string Key; // Convenience typedef
    Key KEY_INPUT = "input";
    Key KEY_INPUT_DEADZONE = "deadzone";
    Key KEY_INPUT_MAXPLAYERS = "max_players";

    Key KEY_AUDIO = "audio";
    Key KEY_AUDIO_FREQ = "freq";
    Key KEY_AUDIO_CHANNELS = "channels";
    Key KEY_AUDIO_CHUNKSIZE = "chunk_size";

    typedef const std::vector<std::string> Keys;
    Keys KEYS_DEADZONE = {KEY_INPUT, KEY_INPUT_DEADZONE};
    Keys KEYS_MAX_PLAYERS = {KEY_INPUT, KEY_INPUT_MAXPLAYERS};
    Keys KEYS_FREQ = {KEY_AUDIO, KEY_AUDIO_FREQ};
    Keys KEYS_CHANNELS = {KEY_AUDIO, KEY_AUDIO_CHANNELS};
    Keys KEYS_CHUNKSIZE = {KEY_AUDIO, KEY_AUDIO_CHUNKSIZE};
}

SettingsSystem::SettingsSystem(std::string config_file)
    : config_file_(config_file)
    , input_settings_()
    , audio_settings_() {
    add_event_handler(EventType::KEYPRESS_EVENT, &SettingsSystem::handle_keypress_event, this);

    if (!reload_settings()) {
        std::cerr << "Unable to load settings. Using defaults..." << std::endl;
    }
}

bool SettingsSystem::reload_settings() {
    std::cout << "Loading settings" << std::endl;

    try {
        head_node_ = YAML::LoadFile(config_file_);
    } catch (YAML::BadFile e) {
        std::cerr << "Failed to load settings file " << config_file_ << std::endl;
        return false;
    }

    bool success = true;
    InputSettings backup_input_settings{input_settings_};
    AudioSettings backup_audio_settings{audio_settings_};

    if (success && head_node_[KEY_INPUT]) {
        if (!reload_input_settings()) {
            std::cerr << "Failed to reload input settings" << std::endl;
            success = false;
        }
    }

    if (success && head_node_[KEY_AUDIO]) {
        if (!reload_audio_settings()) {
            std::cerr << "Failed to reload audio settings" << std::endl;
            success = false;
        }
    }

    if (!success) {
        // restore backups
        std::cerr << "Error reloading settings. Restoring previous config..." << std::endl;
        input_settings_ = std::move(backup_input_settings);
        audio_settings_ = std::move(backup_audio_settings);
    }

    return success;
}

const InputSettings& SettingsSystem::get_input_settings() const {
    return input_settings_;
}

const AudioSettings& SettingsSystem::get_audio_settings() const {
    return audio_settings_;
}

bool SettingsSystem::reload_input_settings() {
    bool success = true;
    success &= load_key<int>(KEYS_DEADZONE, input_settings_.deadzone);
    success &= load_key<int>(KEYS_MAX_PLAYERS, input_settings_.max_players);

    return success;
}

bool SettingsSystem::reload_audio_settings() {
    bool success = true;
    success &= load_key<int>(KEYS_FREQ, audio_settings_.mix_freq_hz);
    success &= load_key<int>(KEYS_CHANNELS, audio_settings_.mix_num_channels);
    success &= load_key<int>(KEYS_CHUNKSIZE, audio_settings_.mix_chunk_size);

    return success;
}

YAML::Node SettingsSystem::load_node(const std::vector<std::string>& keys) const {
    YAML::Node node = YAML::Clone(head_node_);

    for (auto key : keys) {
        node = node[key];
    }

    return node;
}

std::string SettingsSystem::vec_to_str(const std::vector<std::string>& vec) const {
    std::string str = "{";
    auto it = vec.begin();

    if (it != vec.end()) {
        str += *it;
        ++it;
    }

    for (; it != vec.end(); ++it) {
        str += ", " + *it;
    }

    return str + "}";
}

void SettingsSystem::handle_keypress_event(const Event& e) {
    auto key = e.get_value<int>("key", true);

    switch (key.first) {
        case SDLK_F5:
            if (reload_settings()) {
                EventSystem::queue_event( Event( EventType::RELOAD_SETTINGS_EVENT ) );
            }

            break;

        default:
            break;
    }
}

