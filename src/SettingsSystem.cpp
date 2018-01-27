#include "SettingsSystem.h"
#include "SettingType.h"
#include "InputSettings.h"
#include <SDL.h>

SettingsSystem::SettingsSystem(std::string config_file)
    : config_file_(config_file) {
    add_event_handler(EventType::KEYPRESS_EVENT, &SettingsSystem::handle_keypress_event, this);

    if (!reload_settings()) {
        std::cerr << "Unable to load settings" << std::endl;
    }
}

bool SettingsSystem::reload_settings() {
    static const std::string KEY_INPUT = "input";
    std::cout << "Loading settings" << std::endl;

    try {
        head_node_ = YAML::LoadFile(config_file_);
    } catch (YAML::BadFile e) {
        std::cerr << "Failed to load settings file " << config_file_ << std::endl;
        return false;
    }

    if (head_node_[KEY_INPUT]) {
        update_setting(KEY_INPUT, InputSettings::CONFIG_MAPPING);
    }

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
                EventSystem::queue_event( Event( EventType::SETTINGS_EVENT ) );
            }
            break;

        default:
            break;
    }
}

bool SettingsSystem::update_setting(const std::string top_key, const std::map<std::string, std::pair<const SettingType, void* const>>& mappings) {
    for ( const auto& it : mappings ) {
        std::string key = it.first;

        if (head_node_[top_key][key]) {
            auto pair = it.second;
            const SettingType type = pair.first;
            void* const target = pair.second;
            const std::vector<std::string> keys = {top_key, key};

            switch (type) {
                case SettingType::INT:
                    load_key( keys, *static_cast<int* const>( target ) );

                default:
                    std::cerr << "Unknown setting type detected" << std::endl;
                    // return false;
            }
        }
    }

    return true;
}
