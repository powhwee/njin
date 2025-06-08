#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

// inputs
layout (location = 0) in vec3 position;

// outputs
layout (location = 0) out vec4 frag_color;

// model matrix
layout (set = 0, binding = 0) readonly buffer Model {
    mat4 model;
} models[50];

// view-projection matrix
layout (set = 0, binding = 1) uniform VP {
    mat4 view;
    mat4 projection;
} vp;

// push constants
layout (push_constant) uniform ModelIndex {
    int i;
} index;




void main() {
    mat4 model = models[index.i].model;
    gl_Position = transpose(vp.projection) * transpose(vp.view) * transpose(model) * vec4(position, 1.0);
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
