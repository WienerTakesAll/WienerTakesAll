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

    if (shader_ == nullptr || !shader_->is_valid()) {
        std::cerr << "Trying to render with invalid shader!" << std::endl;
        return;
    }


    if (mesh_ == nullptr || !mesh_->valid_) {
        std::cerr << "Trying to render with invalid mesh!" << std::endl;
        return;
    }

    glUseProgram(shader_->get_program_id());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    if (texture_ != nullptr && texture_->is_valid()) {
        glBindTexture(GL_TEXTURE_2D, texture_->get_texture_id());
    }


    GLuint uniformMVP = glGetUniformLocation(shader_->get_program_id(), "MVP");



    for (size_t i = 0; i < mesh_->meshes_.size(); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffers_[i]);
        glVertexAttribPointer
        (0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
         , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, position_)));
        glVertexAttribPointer
        (1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
         , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, normal_)));
        glVertexAttribPointer
        (2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
         , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, uv_)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffers_[i]);


        glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(camera * transform_matrix_));

        glDrawElements(GL_TRIANGLES, mesh_->meshes_[i].indices_.size(), GL_UNSIGNED_INT, 0);
    }

}



void RenderingComponent::apply_transform(glm::mat4x4 transform) {
    transform_matrix_ *= transform;
}

void RenderingComponent::set_transform(glm::mat4x4 transform) {
    transform_matrix_ = transform;
}

const glm::mat4x4& RenderingComponent::get_transform() const {
    return transform_matrix_;
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


    size_t buffer_count = mesh_->meshes_.size();

    if (!buffer_count) {
        return;
    }

    gl_vertex_buffers_.resize(buffer_count);
    gl_index_buffers_.resize(buffer_count);

    glGenBuffers(buffer_count, &gl_vertex_buffers_[0]);
    glGenBuffers(buffer_count, &gl_index_buffers_[0]);

    for (size_t i = 0; i < buffer_count; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffers_[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(MeshAsset::MeshData::VertexData)*mesh_->meshes_[i].vertices_.size(), &mesh_->meshes_[i].vertices_.front(), GL_STATIC_DRAW);


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffers_[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->meshes_[i].indices_.size() * sizeof(GLuint), &mesh_->meshes_[i].indices_.front(), GL_STATIC_DRAW);
    }


}
