#include "SettingsSystem.h"
#include "InputSettings.h"
#include "AudioSettings.h"
#include <SDL.h>
#include <cassert>

namespace {
    typedef std::string Key; // Convenience typedef
    const Key KEY_INPUT = "input";
    const Key KEY_INPUT_DEADZONE = "deadzone";
    const Key KEY_INPUT_MAXPLAYERS = "max_players";

    const Key KEY_AUDIO = "audio";
    const Key KEY_AUDIO_FREQ = "freq";
    const Key KEY_AUDIO_CHANNELS = "channels";
    const Key KEY_AUDIO_CHUNKSIZE = "chunk_size";

    const Key KEY_PHYSICS = "physics";
    const Key KEY_PHYSICS_GRAVITY = "gravity";
    const Key KEY_PHYSICS_PAD_SMOOTHING_DATA = "pad_smoothing_data";
    const Key KEY_PHYSICS_ACCEL_RISE = "accel_rise";
    const Key KEY_PHYSICS_BRAKE_RISE = "brake_rise";
    const Key KEY_PHYSICS_HANDBRAKE_RISE = "handbrake_rise";
    const Key KEY_PHYSICS_STEER_LEFT_RISE = "steer_left_rise";
    const Key KEY_PHYSICS_STEER_RIGHT_RISE = "steer_right_rise";
    const Key KEY_PHYSICS_ACCEL_FALL = "accel_fall";
    const Key KEY_PHYSICS_BRAKE_FALL = "brake_fall";
    const Key KEY_PHYSICS_HANDBRAKE_FALL = "handbrake_fall";
    const Key KEY_PHYSICS_STEER_LEFT_FALL = "steer_left_fall";
    const Key KEY_PHYSICS_STEER_RIGHT_FALL = "steer_right_fall";
    const Key KEY_PHYSICS_STEER_VS_FORWARD_SPEED_DATA = "steer_vs_forward_speed_data";
    const Key KEY_PHYSICS_VEHICLE_MESH = "vehicle_mesh";
    const Key KEY_PHYSICS_ARENA_MESH = "arena_mesh";
    const Key KEY_PHYSICS_ARENA_TIRE_FRICTION = "arena_tire_friction";

    typedef std::vector<Key> Keys;
    const Keys KEYS_DEADZONE = {KEY_INPUT, KEY_INPUT_DEADZONE};
    const Keys KEYS_MAX_PLAYERS = {KEY_INPUT, KEY_INPUT_MAXPLAYERS};

    const Keys KEYS_FREQ = {KEY_AUDIO, KEY_AUDIO_FREQ};
    const Keys KEYS_CHANNELS = {KEY_AUDIO, KEY_AUDIO_CHANNELS};
    const Keys KEYS_CHUNKSIZE = {KEY_AUDIO, KEY_AUDIO_CHUNKSIZE};

    const Keys KEYS_GRAVITY = {KEY_PHYSICS, KEY_PHYSICS_GRAVITY};
    const Keys KEYS_PAD_SMOOTHING = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA};
    const Keys KEYS_ACCEL_RISE = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_ACCEL_RISE};
    const Keys KEYS_BRAKE_RISE = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_BRAKE_RISE};
    const Keys KEYS_HANDBRAKE_RISE = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_HANDBRAKE_RISE};
    const Keys KEYS_STEER_LEFT_RISE = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_STEER_LEFT_RISE};
    const Keys KEYS_STEER_RIGHT_RISE = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_STEER_RIGHT_RISE};
    const Keys KEYS_ACCEL_FALL = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_ACCEL_FALL};
    const Keys KEYS_BRAKE_FALL = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_BRAKE_FALL};
    const Keys KEYS_HANDBRAKE_FALL = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_HANDBRAKE_FALL};
    const Keys KEYS_STEER_LEFT_FALL = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_STEER_LEFT_FALL};
    const Keys KEYS_STEER_RIGHT_FALL = {KEY_PHYSICS, KEY_PHYSICS_PAD_SMOOTHING_DATA, KEY_PHYSICS_STEER_RIGHT_FALL};
    const Keys KEYS_STEER_VS_FORWARD = {KEY_PHYSICS, KEY_PHYSICS_STEER_VS_FORWARD_SPEED_DATA};
    const Keys KEYS_VEHICLE_MESH = {KEY_PHYSICS, KEY_PHYSICS_VEHICLE_MESH};
    const Keys KEYS_ARENA_MESH = {KEY_PHYSICS, KEY_PHYSICS_ARENA_MESH};
    const Keys KEYS_ARENA_TIRE_FRICTION = {KEY_PHYSICS, KEY_PHYSICS_ARENA_TIRE_FRICTION};
}

SettingsSystem::SettingsSystem(std::string config_file)
    : config_file_(config_file)
    , input_settings_()
    , audio_settings_()
    , physics_settings_() {
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
    PhysicsSettings backup_physics_settings{physics_settings_};

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

    if (success && head_node_[KEY_PHYSICS]) {
        if (!reload_physics_settings()) {
            std::cerr << "Failed to reload physics settings" << std::endl;
            success = false;
        }
    }

    if (!success) {
        // restore backups
        std::cerr << "Error reloading settings. Restoring previous config..." << std::endl;
        input_settings_ = std::move(backup_input_settings);
        audio_settings_ = std::move(backup_audio_settings);
        physics_settings_ = std::move(backup_physics_settings);
    }

    return success;
}

const InputSettings& SettingsSystem::get_input_settings() const {
    return input_settings_;
}

const AudioSettings& SettingsSystem::get_audio_settings() const {
    return audio_settings_;
}

const PhysicsSettings& SettingsSystem::get_physics_settings() const {
    return physics_settings_;
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

bool SettingsSystem::reload_physics_settings() {
    bool success = true;
    std::vector<float> gravity;
    success &= load_key<std::vector<float>>(KEYS_GRAVITY, gravity);

    if (success) {
        if (gravity.size() != 3) {
            std::cerr << "Could not parse sequence of size " << gravity.size() << "as PxVec3" << std::endl;
            return false;
        }

        physics_settings_.gravity = {gravity.at(0), gravity.at(1), gravity.at(2)};
    }

    // update smoothing data
    float accel_rise = physics_settings_.g_pad_smoothing_data.mRiseRates[0];
    float brake_rise = physics_settings_.g_pad_smoothing_data.mRiseRates[1];
    float handbrake_rise = physics_settings_.g_pad_smoothing_data.mRiseRates[2];
    float steer_left_rise = physics_settings_.g_pad_smoothing_data.mRiseRates[3];
    float steer_right_rise = physics_settings_.g_pad_smoothing_data.mRiseRates[4];
    float accel_fall = physics_settings_.g_pad_smoothing_data.mFallRates[0];
    float brake_fall = physics_settings_.g_pad_smoothing_data.mFallRates[1];
    float handbrake_fall = physics_settings_.g_pad_smoothing_data.mFallRates[2];
    float steer_left_fall = physics_settings_.g_pad_smoothing_data.mFallRates[3];
    float steer_right_fall = physics_settings_.g_pad_smoothing_data.mFallRates[4];
    success &= load_key<float>(KEYS_ACCEL_RISE, accel_rise);
    success &= load_key<float>(KEYS_BRAKE_RISE, brake_rise);
    success &= load_key<float>(KEYS_HANDBRAKE_RISE, handbrake_rise);
    success &= load_key<float>(KEYS_STEER_LEFT_RISE, steer_left_rise);
    success &= load_key<float>(KEYS_STEER_RIGHT_RISE, steer_right_rise);
    success &= load_key<float>(KEYS_ACCEL_FALL, accel_fall);
    success &= load_key<float>(KEYS_BRAKE_FALL, brake_fall);
    success &= load_key<float>(KEYS_HANDBRAKE_FALL, handbrake_fall);
    success &= load_key<float>(KEYS_STEER_LEFT_FALL, steer_left_fall);
    success &= load_key<float>(KEYS_STEER_RIGHT_FALL, steer_right_fall);

    assert(success);
    physics_settings_.g_pad_smoothing_data = {
        {accel_rise, brake_rise, handbrake_rise, steer_left_rise, steer_right_rise},
        {accel_fall, brake_fall, handbrake_fall, steer_left_fall, steer_right_fall}
    };

    std::vector<float> speed_data;
    success &= load_key<std::vector<float>>(KEYS_STEER_VS_FORWARD, speed_data);

    assert(success);
    assert( speed_data.size() == 16 );

    for (unsigned int index = 0; index < speed_data.size(); ++index) {
        physics_settings_.g_steer_vs_forward_speed_data[index] = speed_data.at(index);
    }

    physics_settings_.g_steer_vs_forward_speed_table = physx::PxFixedSizeLookupTable<8>(physics_settings_.g_steer_vs_forward_speed_data, 4);

    success &= load_key<std::string>(KEYS_VEHICLE_MESH, physics_settings_.vehicle_mesh_asset_path);
    success &= load_key<std::string>(KEYS_ARENA_MESH, physics_settings_.arena_mesh);
    success &= load_key<float>(KEYS_ARENA_TIRE_FRICTION, physics_settings_.arena_tire_friction);

    assert(success);

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
    int value = e.get_value<int>("value", true).first;

    if (value != SDL_KEYDOWN) {
        return;
    }

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

