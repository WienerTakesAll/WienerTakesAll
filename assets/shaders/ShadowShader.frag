#version 330 core

in vec3 norm;

uniform vec3 LightDirection;

out vec4 color;
void main(){
	if(dot(norm,LightDirection) > 0) discard;

	color = vec4(1,1,1,1);
}