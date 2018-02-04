#pragma once

#include <vector>
#include <array>

#include "GL/glew.h"
#include "SDL_opengl.h"
#include "glm/glm.hpp"

class MeshAsset;
class TextureAsset;
class ShaderAsset;

class RenderingComponent {
public:
    RenderingComponent();
    //Renders the object from the camera perspective.
    void render(glm::mat4x4 camera) const;

    void apply_transform(glm::mat4x4 transform);
    void set_transform(glm::mat4x4 transform);

    void set_mesh(MeshAsset* mesh);
    void set_texture(TextureAsset* texture);
    void set_shader(ShaderAsset* shader);

private:
    void setupBuffer();

    GLuint gl_vertex_buffer_;
    GLuint gl_index_buffer_;
    glm::mat4 transform_matrix_;
    MeshAsset* mesh_;
    TextureAsset* texture_;
    ShaderAsset* shader_;
};