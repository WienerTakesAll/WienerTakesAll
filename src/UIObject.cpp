#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "UIObject.h"

UIObject::UIObject(
    glm::vec2 origin,
    glm::vec3 colour,
    glm::vec2 size,
    MeshAsset* mesh,
    TextureAsset* tex,
    ShaderAsset* shader
)
    : visible_(true)
    , origin_(origin)
    , colour_(colour)
    , size_(size)
    , scale_(1.f)
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

void UIObject::set_scale(float factor) {
    scale_ *= factor;
    render_component_.set_transform(glm::scale(glm::mat4(), glm::vec3(scale_, scale_, 0)));
    render_component_.apply_transform(glm::translate(glm::mat4(), glm::vec3(origin_, 0)));
    render_component_.apply_transform(glm::scale(glm::mat4(), glm::vec3(size_, 0)));
}

void UIObject::set_rotation(float angle_radians) {
    render_component_.set_transform(glm::scale(glm::mat4(), glm::vec3(scale_, scale_, 0)));
    render_component_.apply_transform(glm::translate(glm::mat4(), glm::vec3(origin_, 0)));
    render_component_.apply_transform(glm::scale(glm::mat4(), glm::vec3(size_, 0)));
    render_component_.apply_transform(glm::rotate(glm::mat4(), angle_radians, glm::vec3(0.f, 0.f, 1.f)));
}
