#pragma once

#include<yaml-cpp/yaml.h>
#include "EventSystem.h"
#include "SettingType.h"

class SettingsSystem : public EventSystem<SettingsSystem> {
public:
    SettingsSystem(std::string config_file);
    bool reload_settings();

    template<typename T>
    bool load_key(std::vector<std::string> keys, T& value) const {
        YAML::Node node = load_node(keys);

        try {
            value = node.as<T>();
        } catch (YAML::TypedBadConversion<T> e) {
            std::cerr << "Could not load setting " << vec_to_str(keys)
                      << "\nyaml-cpp failed with: " << e.msg;
            return false;
        }

        return true;
    };

private:
    std::string config_file_;
    YAML::Node head_node_;
    YAML::Node load_node(const std::vector<std::string>& keys) const;
    std::string vec_to_str(const std::vector<std::string>& vec) const;
    void handle_keypress_event(const Event& e);
    bool update_setting(const std::string top_key, const std::map<std::string, std::pair<const SettingType, void* const>>& mappings);
};
