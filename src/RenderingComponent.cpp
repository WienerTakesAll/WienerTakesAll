#include "RenderingComponent.h"

#include <iostream>

#include "glm/gtc/type_ptr.hpp"

void RenderingComponent::render_views
(std::array<glm::mat4x4, 4>& cameras, size_t count, GLuint program_id) {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    for (size_t i = 0; i < mesh_->meshes_.size(); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffers_[i]);
        glVertexAttribPointer
        (0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
         , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, position_)));
        glVertexAttribPointer
        (1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VertexData)
         , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VertexData, normal_)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffers_[i]);

        GLuint uniformMVP = glGetUniformLocation(program_id, "MVP");
        GLuint transform = glGetUniformLocation(program_id, "transform");

        glUniformMatrix4fv(transform, 1, GL_FALSE, glm::value_ptr(transform_matrix_));

        for (unsigned int j = 0; j < count; j++) {
            glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(cameras[j] * transform_matrix_));
            glViewport(320 * (j % 2), 240 * ((j % 4) / 2), 320, 240);
            glDrawElements(GL_TRIANGLES, mesh_->meshes_[i].indices_.size(), GL_UNSIGNED_INT, 0);
        }
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