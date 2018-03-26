#version 330 core

in vec3 normal;
in vec2 texCoords;
out vec4 color;

uniform float Ambient;
uniform sampler2D albedo;
uniform vec4 Overlay;

void main(){
	float angle = dot(normalize(vec3(1,1,0)),normalize(normal));
	color.rgb = vec3((0.3-Ambient))*max(0,angle) + vec3(Ambient);
	color.a = 1.0;
	color *= texture(albedo, texCoords);
  color += Overlay;
}
