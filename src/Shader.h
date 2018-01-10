#pragma once

#include <string>

#include <GL\glew.h>
#include "SDL_opengl.h"

class Shader
{
public:
	bool loadShader(const std::string& vertexPath, const std::string& fragmentPath);

	GLuint programID;
private:
	void checkError(GLuint id);
};