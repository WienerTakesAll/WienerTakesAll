#version 330 core

in vec3 vertexColor;
out vec4 color;
void main(){
	color.rgb = vertexColor;
	color.a = 1.0;
}