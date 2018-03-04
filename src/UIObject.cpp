#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "UIObject.h"

UIObject::UIObject(glm::vec2 origin, glm::vec3 colour, glm::vec2 size, MeshAsset* mesh, TextureAsset* tex, ShaderAsset* shader)
    : visible_(true)
    , size_(size)
    , origin_(origin)
    , colour_(colour)
    , render_component_() {
    render_component_.set_mesh(mesh);
    render_component_.set_texture(tex);
    render_component_.set_shader(shader);
    render_component_.apply_transform(glm::translate(glm::mat4(), glm::vec3(origin, 0)));
    render_component_.apply_transform(glm::scale(glm::mat4(), glm::vec3(size, 1)));
}

void UIObject::set_origin(glm::vec2 origin) {
    render_component_.apply_transform(glm::translate(glm::mat4(), glm::vec3(origin, 0)));
}

void UIObject::render(glm::mat4 camera) const {
    if (visible_) {
        render_component_.render(camera);
    }
}

void UIObject::set_texture(TextureAsset* tex) {
    render_component_.set_texture(tex);
}

void UIObject::scale(float factor) {
    render_component_.set_transform(glm::scale(glm::mat4(), glm::vec3(factor, factor, 0)));
    render_component_.apply_transform(glm::translate(glm::mat4(), glm::vec3(origin_, 0)));
    render_component_.apply_transform(glm::scale(glm::mat4(), glm::vec3(size_, 0)));
}
