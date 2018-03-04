#version 330 core

in vec3 vertexColor;
in vec2 textureCoords;

out vec4 color;

uniform sampler2D albedo;

void main(){
	color = texture(albedo, textureCoords);
}