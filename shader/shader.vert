#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

// inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

// outputs
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec3 view_dir;

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
    mat4 model_view = transpose(vp.view) * transpose(model);
    gl_Position = transpose(vp.projection) * model_view * vec4(position, 1.0);

    vec4 eye_pos = model_view * vec4(position, 1.0);
    view_dir = normalize(-eye_pos.xyz);

    frag_normal = mat3(transpose(inverse(model))) * normal;
    frag_color = vec4(0.8, 0.8, 0.8, 1.0);
}