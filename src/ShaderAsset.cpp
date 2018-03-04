#include "ShaderAsset.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

void ShaderAsset::load(const std::string& vertex_path, const std::string& fragment_path) {
    // Create the shaders
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);


    // Read the Vertex Shader code from the file
    std::string vertex_shader_code;
    std::ifstream vertex_shader_stream(vertex_path, std::ios::in);

    if (vertex_shader_stream.is_open()) {
        std::stringstream vertex_shader_string_stream;
        vertex_shader_string_stream << vertex_shader_stream.rdbuf();
        vertex_shader_stream.close();
        vertex_shader_code = vertex_shader_string_stream.str();
    } else {
        std::cerr << "Failed to open " << vertex_path << std::endl;
        valid_ = false;
    }

    // Read the Fragment Shader code from the file
    std::string fragment_shader_code;
    std::ifstream fragment_shader_stream(fragment_path, std::ios::in);

    if (fragment_shader_stream.is_open()) {
        std::stringstream fragment_shader_string_stream;
        fragment_shader_string_stream << fragment_shader_stream.rdbuf();
        fragment_shader_stream.close();
        fragment_shader_code = fragment_shader_string_stream.str();
    } else {
        std::cerr << "Failed to open " << fragment_path << std::endl;
        valid_ = false;
    }


    // Compile Vertex Shader for GPU
    char const* vertex_source_pointer = vertex_shader_code.c_str();
    glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, NULL);
    glCompileShader(vertex_shader_id);

    // Check Vertex Shader
    check_error(vertex_shader_id);

    // Compile Fragment Shader for GPU
    char const* fragment_source_pointer = fragment_shader_code.c_str();
    glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, NULL);
    glCompileShader(fragment_shader_id);

    // Check Fragment Shader
    check_error(fragment_shader_id);



    // Link the program
    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex_shader_id);
    glAttachShader(program_id_, fragment_shader_id);
    glLinkProgram(program_id_);


    glDetachShader(program_id_, vertex_shader_id);
    glDetachShader(program_id_, fragment_shader_id);



    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    valid_ = true;
}

const bool ShaderAsset::is_valid() const {
    return valid_;
}

const GLuint ShaderAsset::get_program_id() const {
    return program_id_;
}

void ShaderAsset::check_error(GLuint id) {
    GLint result = GL_FALSE;
    int length;


    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);


    if (length > 0) {
        std::vector<char> errorMessage(length + 1);
        glGetShaderInfoLog(id, length, NULL, &errorMessage[0]);
        std::cerr << &errorMessage.front();
    }
}
