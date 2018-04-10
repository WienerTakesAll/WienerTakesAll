#pragma once

#include <queue>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "SDL.h"

#include "EventSystem.h"
#include "RenderingComponent.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "ParticleSubsystem.h"

class AssetManager;

class RenderingSystem : public EventSystem<RenderingSystem> {
public:
    RenderingSystem(AssetManager&);

    void update();
    void render();

private:
    void load(const Event& e);
    void handle_add_vehicle(const Event& e);
    void handle_add_terrain(const Event& e);
    void handle_add_charcoal(const Event& e);
    void handle_object_transform(const Event& e);
    void handle_new_it(const Event& e);
    void handle_new_game_state(const Event& e);
    void handle_add_skybox(const Event& e);
    void handle_add_powerup(const Event& e);
    void handle_change_powerup(const Event& e);
    void handle_keypress(const Event& e);
    void handle_use_powerup(const Event& e);
    void handle_finish_powerup(const Event& e);
    void handle_dominate_controls(const Event& e);
    void handle_restore_controls(const Event& e);

    void start_render() const;
    void setup_cameras();
    void end_render() const;

    bool init_window();

    void preload_assets() const;

    SDL_Window* window_;
    GLuint vertex_array_id_;
    std::vector<RenderingComponent> example_objects_;
    std::vector<size_t> car_indices_;
    // FIFO array so the rendered camera lags behind the current car location
    std::queue<std::array<glm::mat4x4, 4>> cameras_queue_;
    std::array<glm::vec3, 4> last_cameras_pos_;

    std::array<float, 4> car_speeds_;

    AssetManager& asset_manager_;
    ShaderAsset* shadow_shader_;
    ParticleSubsystem particle_subsystem_;
    unsigned int whos_it;
};

