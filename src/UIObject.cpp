#include "UIObject.h"

#include <glm/gtc/matrix_transform.hpp>

UIObject::UIObject(glm::vec2 origin, glm::vec3 colour, glm::vec2 size, MeshAsset* mesh, TextureAsset* tex, ShaderAsset* shader)
    : origin_(origin)
    , colour_(colour)
    , size_(size)
    , visible_(true)
    , render_component_() {
    render_component_.set_mesh(mesh);
    render_component_.set_texture(tex);
    render_component_.set_shader(shader);
    render_component_.apply_transform(glm::translate(glm::mat4(), glm::vec3(origin, 0)));
    render_component_.apply_transform(glm::scale(glm::mat4(), glm::vec3(size, 1)));
}

void UIObject::render(glm::mat4 camera) const {
    render_component_.render(camera);
}
