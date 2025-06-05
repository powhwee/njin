#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec2 tex_coord;

// descriptors
layout (set = 2, binding = 0) uniform sampler2D tex_sampler[2];

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform TextureIndex {
    layout (offset = 4) int i;
} index;

void main() {
    //    out_color = texture(tex_sampler[index.i], tex_coord);
    out_color = vec4(1.0, 0.0, 0.0, 0.0);// red
}