#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(AssetManager& asset_manager)
    : asset_manager_(asset_manager) {

}

void LoadingScreen::load() {
    square_mesh_ = asset_manager_.get_mesh_asset("assets/models/UIRect.obj");
    ui_shader_ = asset_manager_.get_shader_asset("assets/shaders/UIShader");
    TextureAsset* heating_bbq_tex =
        asset_manager_.get_texture_asset("assets/textures/heating_bbq.png");
    heating_bbq = UIObject(
                      glm::vec2(-0.35f, -.2f),
                      glm::vec3(1.0f),
                      glm::vec2(.7f),
                      square_mesh_,
                      heating_bbq_tex,
                      ui_shader_
                  );

    TextureAsset* heating_up_bbq_words_tex =
        asset_manager_.get_texture_asset("assets/textures/heating_up_bbq_words.png");
    heating_up_bbq_words = UIObject( // cover screen
                               glm::vec2(-.65f, -1.3f),
                               glm::vec3(1.0f),
                               glm::vec2(1.3f),
                               square_mesh_,
                               heating_up_bbq_words_tex,
                               ui_shader_
                           );
}

void LoadingScreen::render() const {
    heating_bbq.render(glm::mat4());
    heating_up_bbq_words.render(glm::mat4());
}
