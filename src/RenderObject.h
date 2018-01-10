#include <vector>
#include <array>

#include <GL\glew.h>
#include "SDL_opengl.h"
#include <glm\glm.hpp>

class RenderObject
{
public:
	//Takes a list of camera matrices and renders up to 4 views
	//Does not set the shader
	void renderViews(std::array<glm::mat4x4,4>& cameras, size_t count, GLuint programID);
	void loadFromFile(const std::string& filePath);

private:
	void setupBuffer();

	struct VertexData
	{
		float position[3];
		float normal[3];
		float colors[3];
		float uv[2];
	};

	GLuint glVertexBuffer, glIndexBuffer;
	std::vector<VertexData> vertices;
	std::vector<GLuint> indices;
};