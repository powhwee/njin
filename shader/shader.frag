#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

layout (location = 0) in vec4 frag_color;

layout (location = 0) out vec4 out_color;

void main() {
    out_color = vec4(1.0);
}