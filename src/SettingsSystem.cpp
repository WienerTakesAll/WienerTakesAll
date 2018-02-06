#include "SettingsSystem.h"
#include "InputSettings.h"
#include "AudioSettings.h"
#include <SDL.h>

namespace {
    typedef const std::string Key; // Convenience typedef
    Key KEY_INPUT = "input";
    Key KEY_INPUT_DEADZONE = "deadzone";

    Key KEY_AUDIO = "audio";
}

SettingsSystem::SettingsSystem(std::string config_file)
    : config_file_(config_file) {
    add_event_handler(EventType::KEYPRESS_EVENT, &SettingsSystem::handle_keypress_event, this);
    input_settings_ = std::make_shared<InputSettings>();
    audio_settings_ = std::make_shared<AudioSettings>();

    if (!reload_settings()) {
        std::cerr << "Unable to load settings" << std::endl;
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

    if (head_node_[KEY_INPUT]) {
        if (!reload_input_settings()) {
            std::cerr << "Failed to reload input settings" << std::endl;
            return false;
        }
    }

    if (head_node_[KEY_AUDIO]) {
        if (!reload_audio_settings()) {
            std::cerr << "Failed to reload audio settings" << std::endl;
            return false;
        }
    }

    return true;
}

std::shared_ptr<InputSettings> SettingsSystem::get_input_settings() {
    return input_settings_;
}

std::shared_ptr<AudioSettings> SettingsSystem::get_audio_settings() {
    return audio_settings_;
}

bool SettingsSystem::reload_input_settings() {
    auto deadzone_node = load_node({KEY_INPUT, KEY_INPUT_DEADZONE});

    if (deadzone_node) {
        try {
            input_settings_->deadzone = deadzone_node.as<int>();
        } catch (YAML::TypedBadConversion<int> e) {
            std::cerr << "Error: failed to read setting {input, deadzone} as the value was not an int" << std::endl;
            return false;
        }
    }

    return true;
}

bool SettingsSystem::reload_audio_settings() {
    // Add hot-reloadable audio settings here
    return true;
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
    int key = e.get_value<int>("key", -1);

    switch (key) {
        case SDLK_F5:
            if (reload_settings()) {
                EventSystem::queue_event( Event( EventType::RELOAD_SETTINGS_EVENT ) );
            }

            break;

        default:
            break;
    }
}

