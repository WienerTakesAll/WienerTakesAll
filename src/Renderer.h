#pragma once

#include "EventSystem.h"
#include "RenderObject.h"
#include "Shader.h"

#include "SDL_opengl.h"
#include "SDL.h"

class Renderer : public EventSystem<Renderer>
{
public:
	Renderer();
	
	void update();
	void render();
private:
	void load(Event e);

	void startRender();
	void setupCameras();
	void endRender();

	bool initWindow();
	SDL_Window* window;

	GLuint VertexArrayID;
	Shader exampleShader;
	RenderObject exampleObject;
	float time;
	std::array<glm::mat4x4, 4> cameras;
};

