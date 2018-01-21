#include "Shader.h"

#include <fstream>
#include <iostream>
#include <vector>

bool Shader::load_shader(const std::string& vertex_path, const std::string& fragment_path) {
    // Create the shaders
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    std::string VertexShaderCode;

    std::ifstream VertexShaderStream(vertex_path, std::ios::in);

    if (VertexShaderStream.is_open()) {
        std::string Line = "";

        while (getline(VertexShaderStream, Line)) {
            VertexShaderCode += "\n" + Line;
        }

        VertexShaderStream.close();
    } else {
        std::cout << "Failed to open " << vertex_path << std::endl;
        return false;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_path, std::ios::in);

    if (FragmentShaderStream.is_open()) {
        std::string Line = "";

        while (getline(FragmentShaderStream, Line)) {
            FragmentShaderCode += "\n" + Line;
        }

        FragmentShaderStream.close();
    }



    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(vertex_shader_id, 1, &VertexSourcePointer, NULL);
    glCompileShader(vertex_shader_id);

    check_error(vertex_shader_id);

    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(fragment_shader_id, 1, &FragmentSourcePointer, NULL);
    glCompileShader(fragment_shader_id);

    // Check Fragment Shader
    check_error(fragment_shader_id);

    // Link the program
    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex_shader_id);
    glAttachShader(program_id_, fragment_shader_id);
    glLinkProgram(program_id_);


    check_error(program_id_);

    glDetachShader(program_id_, vertex_shader_id);
    glDetachShader(program_id_, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);


    return true;
}

void Shader::check_error(GLuint id) {
    GLint result = GL_FALSE;
    int length;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

    if (length > 0) {
        std::vector<char> errorMessage(length + 1);
        glGetShaderInfoLog(id, length, NULL, &errorMessage[0]);
        std::cout << &errorMessage.front();
    }
}