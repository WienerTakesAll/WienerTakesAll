#version 330 core
layout(location = 0) in vec4 vertexPosition_modelspace;
layout(location = 1) in vec3 normal;

out vec3 norm;

uniform mat4 Model;
uniform mat4 View;
uniform vec3 LightDirection;

void main(){
  vec3 toInf = LightDirection*vertexPosition_modelspace.w*2048*2048;
  vec4 modelPos = Model*vec4(vertexPosition_modelspace.xyz, 1.0) + vec4(toInf,0);
  gl_Position = View * modelPos;
  norm = (Model * vec4(normal,0.0)).xyz;
}