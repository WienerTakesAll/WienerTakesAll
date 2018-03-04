#pragma once

#include <vector>
#include <array>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "glm/glm.hpp"

struct MeshAsset;
class TextureAsset;
class ShaderAsset;

class RenderingComponent {
public:
    RenderingComponent();
    //Renders the object from the camera perspective.
    void render(glm::mat4x4 camera, float ambient = 0.f) const;
    void render_lighting(glm::mat4x4 camera, glm::vec3 light_direction, ShaderAsset* shadow_shader) const;
    // Multiplies current transform with new transform
    void apply_transform(glm::mat4x4 transform);
    // Sets current transform to new transform
    void set_transform(glm::mat4x4 transform);
    void set_has_shadows(bool has_shadow);
    const glm::mat4x4& get_transform() const;

    void set_mesh(MeshAsset* mesh);
    void set_texture(TextureAsset* texture);
    void set_shader(ShaderAsset* shader);

private:
    void setupBuffer();

    std::vector<GLuint> gl_vertex_buffers_;
    std::vector<GLuint> gl_index_buffers_;


    std::vector<GLuint> gl_shadow_vertex_buffers_;
    std::vector<GLuint> gl_shadow_index_buffers_;

    glm::mat4 transform_matrix_;
    MeshAsset* mesh_;
    TextureAsset* texture_;
    ShaderAsset* shader_;
    bool has_shadows_;
};
