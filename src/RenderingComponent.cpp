#include "RenderingComponent.h"

#include <iostream>

#include "glm/gtc/type_ptr.hpp"

void RenderingComponent::render_views
(std::array<glm::mat4x4, 4>& cameras, size_t count, GLuint program_id) {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_);
    glVertexAttribPointer
    (0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
     , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, position_)));
    glVertexAttribPointer
    (1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
     , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, normal_)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_);

    GLuint uniformMVP = glGetUniformLocation(program_id, "MVP");
    GLuint transform = glGetUniformLocation(program_id, "transform");

    glUniformMatrix4fv(transform, 1, GL_FALSE, glm::value_ptr(transform_matrix_));

    for (unsigned int i = 0; i < count; i++) {
        glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(cameras[i] * transform_matrix_));
        glViewport(320 * (i % 2), 240 * ((i % 4) / 2), 320, 240);
        glDrawElements(GL_TRIANGLES, mesh_->indices_.size(), GL_UNSIGNED_INT, 0);
    }
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

void RenderingComponent::setupBuffer() {
    glGenBuffers(1, &gl_vertex_buffer_);
    glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshAsset::VertexData)*mesh_->vertices_.size(), &mesh_->vertices_.front(), GL_STATIC_DRAW);

    glGenBuffers(1, &gl_index_buffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_->indices_.size() * sizeof(GLuint), &mesh_->indices_.front(), GL_STATIC_DRAW);

}