#version 450
layout (location = 0) in vec3 frag_color;

layout (location = 0) out vec3 out_color;

void main() {
	out_color = frag_color;
}