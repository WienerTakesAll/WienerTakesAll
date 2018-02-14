#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 normals;
out vec3 vertexColor;

uniform mat4 MVP;

void main(){
  gl_Position = MVP * vec4(vertexPosition_modelspace,1.0);
  vertexColor = vec3(vertexPosition_modelspace.x, vertexPosition_modelspace.y, vertexPosition_modelspace.z);
}