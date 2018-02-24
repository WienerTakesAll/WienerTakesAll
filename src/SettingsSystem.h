#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "EventSystem.h"

struct InputSettings;
struct AudioSettings;

class SettingsSystem : public EventSystem<SettingsSystem> {
public:
    SettingsSystem(std::string config_file);
    bool reload_settings();

    std::shared_ptr<InputSettings> get_input_settings();
    std::shared_ptr<AudioSettings> get_audio_settings();

    template<typename T>
    bool load_key(std::vector<std::string> keys, T& value) const {
        YAML::Node node = load_node(keys);

        if (node) {
            try {
                value = node.as<T>();
            } catch (YAML::TypedBadConversion<T> e) {
                std::cerr << "Could not load setting " << vec_to_str(keys)
                          << "\nyaml-cpp failed with: " << e.msg << std::endl;
                return false;
            }
        } else {
            std::cout << "Node " << vec_to_str(keys) << "was not present in config file." << std::endl;
        }

        return true;
    };

private:
    std::string config_file_;
    YAML::Node head_node_;
    std::shared_ptr<InputSettings> input_settings_;
    std::shared_ptr<AudioSettings> audio_settings_;

    bool reload_input_settings();
    bool reload_audio_settings();

    YAML::Node load_node(const std::vector<std::string>& keys) const;
    std::string vec_to_str(const std::vector<std::string>& vec) const;
    void handle_keypress_event(const Event& e);
};
