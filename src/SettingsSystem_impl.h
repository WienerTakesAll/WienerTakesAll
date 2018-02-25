template<typename T>
bool SettingsSystem::load_key(std::vector<std::string> keys, T& value) {
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

