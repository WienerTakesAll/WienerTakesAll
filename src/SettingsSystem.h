#pragma once

#include <memory>
#include <yaml-cpp/yaml.h>

#include "EventSystem.h"
#include "AudioSettings.h"
#include "InputSettings.h"


class SettingsSystem : public EventSystem<SettingsSystem> {
public:
    SettingsSystem(std::string config_file);
    bool reload_settings();

    const InputSettings& get_input_settings() const;
    const AudioSettings& get_audio_settings() const;

    template<typename T>
    bool load_key(std::vector<std::string> keys, T& value);

private:
    std::string config_file_;
    YAML::Node head_node_;
    InputSettings input_settings_;
    AudioSettings audio_settings_;

    bool reload_input_settings();
    bool reload_audio_settings();

    YAML::Node load_node(const std::vector<std::string>& keys) const;
    std::string vec_to_str(const std::vector<std::string>& vec) const;
    void handle_keypress_event(const Event& e);
};

#include "SettingsSystem_impl.h"
