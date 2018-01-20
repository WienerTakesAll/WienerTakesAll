#include "SettingsSystem.h"

SettingsSystem::SettingsSystem(std::string config_file)
    : config_file_(config_file) {
    if (!reload_settings()) {
        std::cerr << "Unable to load settings" << std::endl;
    }
}

bool SettingsSystem::reload_settings() {
    try {
        head_node_ = YAML::LoadFile(config_file_);
    } catch (YAML::BadFile e) {
        std::cerr << "Failed to load settings file " << config_file_ << std::endl;
    }

    return true;
}

YAML::Node SettingsSystem::load_node(std::vector<std::string> keys) const {
    YAML::Node node = YAML::Clone(head_node_);

    for (auto key : keys) {
        node = node[key];
    }

    return node;
}

std::string SettingsSystem::vec_to_str(std::vector<std::string> vec) const {
    std::string str = "{";
    auto it = vec.begin();

    if (it != vec.end()) {
        str += *it;
    }

    for (; it != vec.end(); ++it) {
        str += ", " + *it;
    }

    return str + "}";
}
