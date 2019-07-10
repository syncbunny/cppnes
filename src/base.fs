#version 150 core

uniform sampler2D image;
in vec2 texcoord;
out vec4 fragment;

void main() {
	fragment = texture(image, texcoord);
}
