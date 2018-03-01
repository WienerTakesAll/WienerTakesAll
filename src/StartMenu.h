#pragma once

#include "UIObject.h"
#include "AssetManager.h"

class StartMenu {
public:
	StartMenu(AssetManager& asset_manager);
	void load();
	void render() const;

private:
    AssetManager& asset_manager_;

    MeshAsset* square_mesh_;
    ShaderAsset* ui_shader_;

    UIObject background_;
    UIObject logo_;
    UIObject hit_enter_or_start_;
};