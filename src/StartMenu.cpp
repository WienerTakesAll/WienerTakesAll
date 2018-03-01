#include "StartMenu.h"

StartMenu::StartMenu(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {
}

void StartMenu::load() {
    square_mesh_ = asset_manager_.get_mesh_asset("assets/models/UIRect.obj");
    ui_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* background_tex =
        asset_manager_.get_texture_asset("assets/textures/backyard_bbq.png");
    background_ = UIObject( // cover screen
                    glm::vec2(-1.f),
                    glm::vec3(1.0f),
                    glm::vec2(2.0f),
                    square_mesh_,
                    background_tex,
                    ui_shader_
                );

    TextureAsset* logo_tex =
        asset_manager_.get_texture_asset("assets/textures/logo.png");
    logo_ = UIObject( // top middle
                glm::vec2(-0.5f, -.1f),
                glm::vec3(1.0f),
                glm::vec2(1.0f),
                square_mesh_,
                logo_tex,
                ui_shader_
            );

    TextureAsset* hit_enter_or_start_tex =
        asset_manager_.get_texture_asset("assets/textures/hit_enter_or_start.png");
    hit_enter_or_start_ = UIObject( // top middle
                              glm::vec2(-.9f, -1.3f),
                              glm::vec3(1.0f),
                              glm::vec2(1.8f),
                              square_mesh_,
                              hit_enter_or_start_tex,
                              ui_shader_
                          );
}

void StartMenu::render() const {
	background_.render(glm::mat4());
	logo_.render(glm::mat4());
	hit_enter_or_start_.render(glm::mat4());
}

