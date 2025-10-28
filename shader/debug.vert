#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

// inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec2 tex_coord;
layout (location = 4) in vec4 color;

// outputs
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec2 frag_tex_coord;

// model matrix
layout (set = 0, binding = 0) readonly buffer Model {
    mat4 model;
} models[16];

// view-projection matrix
layout (set = 0, binding = 1) uniform VP {
    mat4 view;
    mat4 projection;
} vp;

// push constants
layout (push_constant) uniform ModelIndex {
    int i;
} index;


vec3 srgb_to_linear(vec3 srgb_color) {
    return pow(srgb_color, vec3(2.2));
}


void main() {
    mat4 model = models[index.i].model;
    gl_Position = transpose(vp.projection) * transpose(vp.view) * transpose(model) * vec4(position, 1.0);
    frag_color = color;
    frag_tex_coord = tex_coord;
}
