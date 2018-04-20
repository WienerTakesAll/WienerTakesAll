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

	// https://www.cs.rpi.edu/~cutler/classes/advancedgraphics/S12/final_projects/hutchins_kim.pdf
	if(angle > 0.5){
		color *= vec4(1.0);
	}

	else if(angle > 0.1) {
		color *= vec4(0.8, 0.8, 0.62, 1.0);
	}

	else {
		color *= vec4(0.43, 0.4, 0.31, 1.0); 
	}

	color *= texture(albedo, texCoords);
	color += Overlay;
}
