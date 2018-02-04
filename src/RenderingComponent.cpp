#include "RenderingComponent.h"

#include <iostream>

#include "glm/gtc/type_ptr.hpp"

#include "MeshAsset.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"

RenderingComponent::RenderingComponent()
    : mesh_(nullptr),
      texture_(nullptr),
      shader_(nullptr) {
}

void RenderingComponent::render(glm::mat4x4 camera) const {

    if (shader_ == nullptr || !shader_->valid_) {
        std::cerr << "Trying to render with invalid shader!" << std::endl;
        return;
    }


    if (mesh_ == nullptr || !mesh_->valid_) {
        std::cerr << "Trying to render with invalid mesh!" << std::endl;
        return;
    }

    glUseProgram(shader_->program_id_);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_);
    glVertexAttribPointer
    (0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
     , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, position_)));
    glVertexAttribPointer
    (1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
     , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, normal_)));
    glVertexAttribPointer
    (2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
     , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, uv_)));


    if (texture_ != nullptr && texture_->valid_) {
        glBindTexture(GL_TEXTURE_2D, texture_->texture_id_);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_);

    GLuint uniformMVP = glGetUniformLocation(shader_->program_id_, "MVP");
    GLuint transform = glGetUniformLocation(shader_->program_id_, "transform");

    glUniformMatrix4fv(transform, 1, GL_FALSE, glm::value_ptr(transform_matrix_));

    glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(camera * transform_matrix_));
    glDrawElements(GL_TRIANGLES, mesh_->indices_.size(), GL_UNSIGNED_INT, 0);
}



void RenderingComponent::apply_transform(glm::mat4x4 transform) {
    transform_matrix_ *= transform;
}

void RenderingComponent::set_transform(glm::mat4x4 transform) {
    transform_matrix_ = transform;
}



void RenderingComponent::set_mesh(MeshAsset* mesh) {
    mesh_ = mesh;
    setupBuffer();
}

void RenderingComponent::set_texture(TextureAsset* texture) {
    texture_ = texture;
}

void RenderingComponent::set_shader(ShaderAsset* shader) {
    shader_ = shader;
}

void RenderingComponent::setupBuffer() {

    if (mesh_ != nullptr && mesh_->valid_) {
        glGenBuffers(1, &gl_vertex_buffer_);
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(MeshAsset::VertexData)*mesh_->vertices_.size(), &mesh_->vertices_.front(), GL_STATIC_DRAW);

        glGenBuffers(1, &gl_index_buffer_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->indices_.size() * sizeof(GLuint), &mesh_->indices_.front(), GL_STATIC_DRAW);
    }

}