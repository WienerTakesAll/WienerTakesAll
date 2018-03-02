#version 330 core

in vec3 normal;
out vec4 color;

uniform float Ambient;

void main(){
	float angle = dot(normalize(vec3(1,1,0)),normalize(normal));
	color.rgb = vec3((0.3-Ambient)/0.3)*max(0,angle) + vec3(Ambient);
	color.a = 1.0;
}