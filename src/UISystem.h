#pragma once

#include <vector>

#include "EventSystem.h"
#include "Shader.h"
#include "UIObject.h"

#include "SDL.h"
#include "SDL_opengl.h"

class AssetManager;

class UISystem : public EventSystem<UISystem> {
public:
    UISystem(AssetManager&);
    void update();
    void render() const;

private:
    void start_render() const;
    void end_render() const;

    void handle_load(const Event& e);
    void handle_key_press(const Event& e);

    AssetManager& asset_manager_;
    UIObject* active_button_;
    Shader example_shader_;
    std::vector<UIObject> ui_objects_;
};
