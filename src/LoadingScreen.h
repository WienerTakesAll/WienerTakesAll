#pragma once

#include "UIObject.h"
#include "AssetManager.h"

class LoadingScreen {
public:
    LoadingScreen(AssetManager& asset_manager);
    void load();
    void render() const;

private:
    AssetManager& asset_manager_;

    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject heating_bbq;
    UIObject heating_up_bbq_words;
};
