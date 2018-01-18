#include "RenderingComponent.h"

#include <iostream>

#include "glm/gtc/type_ptr.hpp"

void RenderingComponent::render_views
(std::array<glm::mat4x4, 4>& cameras, size_t count, GLuint program_id) {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
    glVertexAttribPointer
    (0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
        , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, position)));
    glVertexAttribPointer
    (1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::VertexData)
        , reinterpret_cast<void*>(offsetof(MeshAsset::VertexData, normal)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer);

    GLuint uniformMVP = glGetUniformLocation(program_id, "MVP");

    for (unsigned int i = 0; i < count; i++) {
        glUniformMatrix4fv(uniformMVP, 1, GL_FALSE, glm::value_ptr(cameras[i] * transform_matrix));
        glViewport(320 * (i % 2), 240 * ((i % 4) / 2), 320, 240);
        glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
    }



}



void RenderingComponent::apply_transform(glm::mat4x4 transform) {
    transform_matrix *= transform;
}

void RenderingComponent::set_transform(glm::mat4x4 transform) {
    transform_matrix = transform;
}



void RenderingComponent::set_mesh(MeshAsset* n_mesh) {
    mesh = n_mesh;
    setupBuffer();
}

void RenderingComponent::setupBuffer() {
    glGenBuffers(1, &gl_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(MeshAsset::VertexData)*mesh->vertices.size(), &mesh->vertices.front(), GL_STATIC_DRAW);

    glGenBuffers(1, &gl_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(GLuint), &mesh->indices.front(), GL_STATIC_DRAW);

}