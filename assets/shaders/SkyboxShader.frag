#version 330 core

in vec2 textureCoord;
out vec4 color;

uniform float Ambient;
uniform sampler2D albedo;

void main(){
	color.rgba = texture2D(albedo,textureCoord);
}