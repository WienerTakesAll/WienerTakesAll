#pragma once

#include<yaml-cpp/yaml.h>
#include "EventSystem.h"

class SettingsSystem : public EventSystem<SettingsSystem> {
public:
    SettingsSystem(std::string config_file);
    bool reload_settings();

    template<typename T>
    bool load_keys(std::vector<std::string> keys, T& value) const {
        YAML::Node node = load_node(keys);

        try {
            value = node.as<T>();
        } catch (YAML::InvalidNode e) {
            std::cerr << "Could not load setting " << vec_to_str(keys)
                      << "\nyaml-cpp failed with: " << e.msg();
            return false;
        }

        return true;
    };

private:
    std::string config_file_;
    YAML::Node head_node_;
    YAML::Node load_node(std::vector<std::string> keys) const;
    std::string vec_to_str(std::vector<std::string> vec) const;
};
