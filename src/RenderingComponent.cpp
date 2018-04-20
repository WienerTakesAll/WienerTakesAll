#include "RenderingComponent.h"

#include <iostream>

#include "glm/gtc/type_ptr.hpp"

#include "MeshAsset.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"

RenderingComponent::RenderingComponent()
    : mesh_(nullptr)
    , shadow_mesh_(nullptr)
    , texture_(nullptr)
    , shader_(nullptr)
    , has_shadows_(false) {
}

void RenderingComponent::render(glm::mat4x4 camera, float ambient) const {

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
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }



    GLuint uniform_model = glGetUniformLocation(shader_->get_program_id(), "Model");
    GLuint uniform_view = glGetUniformLocation(shader_->get_program_id(), "View");
    GLuint uniform_ambient = glGetUniformLocation(shader_->get_program_id(), "Ambient");
    GLuint uniform_overlay = glGetUniformLocation(shader_->get_program_id(), "Overlay");

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


        glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(transform_matrix_));
        glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(camera));
        glUniform4fv(uniform_overlay, 1, glm::value_ptr(colour_overlay_));
        glUniform1f(uniform_ambient, ambient);

        glDrawElements(GL_TRIANGLES, mesh_->meshes_[i].indices_.size(), GL_UNSIGNED_INT, 0);
    }

}

void RenderingComponent::render_lighting(glm::mat4x4 camera, glm::vec3 light_direction, ShaderAsset* shadow_shader) const {
    if (!has_shadows_ || !shadow_mesh_) {
        return;
    }

    if (shadow_shader == nullptr || !shadow_shader->is_valid()) {
        std::cerr << "Trying to render with invalid shader!" << std::endl;
        return;
    }


    if (shadow_mesh_ == nullptr || !shadow_mesh_->valid_) {
        std::cerr << "Trying to render with invalid mesh!" << std::endl;
        return;
    }


    glUseProgram(shadow_shader->get_program_id());

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    GLuint uniform_model = glGetUniformLocation(shadow_shader->get_program_id(), "Model");
    GLuint uniform_view = glGetUniformLocation(shadow_shader->get_program_id(), "View");
    GLuint uniform_light = glGetUniformLocation(shadow_shader->get_program_id(), "LightDirection");


    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LESS);

    glStencilFunc(GL_ALWAYS, 0, ~0);
    glStencilMask(~0);


    //Draw the front stencil shadow mask
    glCullFace(GL_BACK);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);


    glDisable(GL_CULL_FACE);

    for (size_t i = 0; i < shadow_mesh_->meshes_.size(); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, gl_shadow_vertex_buffers_[i]);
        glVertexAttribPointer
        ( 0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VolumeVertexData)
          , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VolumeVertexData, position_)));
        glVertexAttribPointer
        ( 1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshAsset::MeshData::VolumeVertexData)
          , reinterpret_cast<void*>(offsetof(MeshAsset::MeshData::VolumeVertexData, normal_)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_shadow_index_buffers_[i]);

        glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(transform_matrix_));
        glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(camera));
        glUniform3f(uniform_light, light_direction.x, light_direction.y, light_direction.z);

        glDrawElements(GL_TRIANGLES, shadow_mesh_->meshes_[i].shadow_volume_indices_.size(), GL_UNSIGNED_INT, 0);
    }

    glCullFace(GL_BACK);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glDisable(GL_CULL_FACE);

}

void RenderingComponent::render_outline(glm::mat4x4 camera, ShaderAsset* outline_shader) const {
    if (outline_shader == nullptr || !outline_shader->is_valid()) {
        std::cerr << "Trying to render with invalid shader!" << std::endl;
        return;
    }

    if (mesh_ == nullptr || !mesh_->valid_) {
        std::cerr << "Trying to render with invalid mesh!" << std::endl;
        return;
    }

    if (texture_ != nullptr && texture_->is_valid()) {
        glBindTexture(GL_TEXTURE_2D, texture_->get_texture_id());
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    // https://www.codeproject.com/Articles/8499/Generating-Outlines-in-OpenGL
    glUseProgram(outline_shader->get_program_id());

    // Enable polygon offsets, and offset filled polygons forward by 2.5
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( -5.5f, -5.5f );

    // Set the render mode to be line rendering with a thick line width
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glLineWidth( 2.0f );

    // Render the object

    GLuint uniform_model = glGetUniformLocation(shader_->get_program_id(), "Model");
    GLuint uniform_view = glGetUniformLocation(shader_->get_program_id(), "View");

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

        glUniformMatrix4fv(uniform_model, 1, GL_FALSE, glm::value_ptr(transform_matrix_));
        glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(camera));

        glDrawElements(GL_TRIANGLES, mesh_->meshes_[i].indices_.size(), GL_UNSIGNED_INT, 0);
    }

    // Set the polygon mode to be filled triangles
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    // glDisable(GL_POLYGON_OFFSET_FILL);

    // Pop the state changes off the attribute stack
    // to set things back how they were
    glPopAttrib();
}

void RenderingComponent::apply_transform(glm::mat4x4 transform) {
    transform_matrix_ *= transform;
}

void RenderingComponent::set_transform(glm::mat4x4 transform) {
    transform_matrix_ = transform;
}

void RenderingComponent::set_colour_overlay(glm::vec4 colour) {
    colour_overlay_ = colour;
}

void RenderingComponent::set_has_shadows(bool has_shadows) {
    has_shadows_ = has_shadows;
}

const glm::mat4x4& RenderingComponent::get_transform() const {
    return transform_matrix_;
}


void RenderingComponent::set_mesh(MeshAsset* mesh) {
    mesh_ = mesh;
    setupBuffer();
}

void RenderingComponent::set_shadow_mesh(MeshAsset* shadow_mesh) {
    shadow_mesh_ = shadow_mesh;
    setupShadowBuffer();
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


void RenderingComponent::setupShadowBuffer() {

    size_t buffer_count = shadow_mesh_->meshes_.size();

    if (!buffer_count) {
        return;
    }

    gl_shadow_vertex_buffers_.resize(buffer_count);
    gl_shadow_index_buffers_.resize(buffer_count);

    glGenBuffers(buffer_count, &gl_shadow_vertex_buffers_[0]);
    glGenBuffers(buffer_count, &gl_shadow_index_buffers_[0]);


    for (size_t i = 0; i < buffer_count; i++) {

        if (shadow_mesh_ && shadow_mesh_->meshes_[i].shadow_volume_vertices_.size() && shadow_mesh_->meshes_[i].shadow_volume_indices_.size()) {
            glBindBuffer(GL_ARRAY_BUFFER, gl_shadow_vertex_buffers_[i]);
            glBufferData
            (GL_ARRAY_BUFFER, sizeof(MeshAsset::MeshData::VolumeVertexData)*shadow_mesh_->meshes_[i].shadow_volume_vertices_.size()
             , &shadow_mesh_->meshes_[i].shadow_volume_vertices_.front(), GL_STATIC_DRAW);


            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_shadow_index_buffers_[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadow_mesh_->meshes_[i].shadow_volume_indices_.size() * sizeof(GLuint), &shadow_mesh_->meshes_[i].shadow_volume_indices_.front(), GL_STATIC_DRAW);
        }

    }



}
