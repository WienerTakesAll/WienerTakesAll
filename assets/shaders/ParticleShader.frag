#version 330 core

in vec2 texCoord;

uniform sampler2D tex;

out vec4 color;
void main(){
	color = texture2D(tex,texCoord);
	if (color.a <= 0.0) {
		discard;
	}
}
