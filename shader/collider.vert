#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

// inputs
layout (location = 0) in vec3 position;

// outputs
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec2 out_tex_coords;

// model matrix
layout (set = 0, binding = 0) readonly buffer Transform {
    mat4 transform;
} transforms[100];

// view-projection matrix
layout (set = 0, binding = 1) uniform VP {
    mat4 view;
    mat4 projection;
} vp;

// push constants
layout (push_constant) uniform TransformIndex {
    int i;
} index;

void main() {
    mat4 transform = transforms[index.i].transform;
    gl_Position = transpose(vp.projection) * transpose(vp.view) * transpose(transform) * vec4(position, 1.0);
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
