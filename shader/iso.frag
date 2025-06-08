#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec2 tex_coords;

// descriptors
layout (set = 1, binding = 0) uniform sampler2D tex_sampler[10];

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform TextureIndex {
    layout (offset = 4) int i;
} texture_index;


void main() {
    vec4 colors = texture(tex_sampler[texture_index.i], tex_coords);
    out_color = colors;
}
