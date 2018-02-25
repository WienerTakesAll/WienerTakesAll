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
    bool load_key(std::vector<std::string> keys, T& value);

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

#include "SettingsSystem_impl.h"
