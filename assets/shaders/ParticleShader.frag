#version 330 core

in vec2 texCoord;

uniform sampler2D tex;
uniform vec4 Overlay;

out vec4 color;
void main(){
    color = texture(tex,texCoord);
    color += Overlay;

    if (color.a <= 0.0) {
        discard;
    }
}
