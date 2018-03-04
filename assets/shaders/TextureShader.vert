#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 uv;

out vec3 vertexColor;
out vec2 textureCoords;

uniform mat4 MVP;

void main(){
  gl_Position = MVP * vec4(vertexPosition_modelspace,1.0);
  vertexColor = vec3(vertexPosition_modelspace.x, vertexPosition_modelspace.y, vertexPosition_modelspace.z);
  textureCoords = uv;
}