#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 uv;

out vec2 texCoord;

uniform mat4 Model;
uniform mat4 View;

void main(){
  gl_Position = View * Model * vec4(vertexPosition_modelspace,1.0);
  texCoord = uv;
}