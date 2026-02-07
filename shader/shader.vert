#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

// inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 tangent;
layout (location = 3) in vec2 tex_coord;
layout (location = 4) in vec4 color;
layout (location = 5) in uvec4 joints;
layout (location = 6) in vec4 weights;

// outputs
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec3 frag_normal;
layout (location = 2) out vec3 frag_tangent;
layout (location = 3) out vec3 frag_bitangent;
layout (location = 4) out vec3 view_dir;
layout (location = 5) out vec2 frag_tex_coord;

// model matrix
layout (set = 0, binding = 0) readonly buffer Model {
    mat4 model;
} models[1024];

// view-projection matrix
layout (set = 0, binding = 1) uniform VP {
    mat4 view;
    mat4 projection;
} vp;

// joint matrices for GPU skinning
layout (set = 0, binding = 2) readonly buffer Joints {
    mat4 joint;
} joint_matrices[128];

// push constants
layout (push_constant) uniform PushConstants {
    int model_index;
    int texture_index;  // unused in vertex shader but must match layout
    float base_color_r;
    float base_color_g;
    float base_color_b;
    float base_color_a;
    int joint_offset;   // offset into joint_matrices SSBO (-1 if not skinned)
    int joint_count;    // number of joints (0 if not skinned)
} pc;


vec3 srgb_to_linear(vec3 srgb_color) {
    return pow(srgb_color, vec3(2.2));
}


void main() {
    mat4 model = models[pc.model_index].model;

    vec4 skinned_pos;
    vec3 skinned_normal;
    vec4 skinned_tangent;

    if (pc.joint_count > 0 && pc.joint_offset >= 0) {
        // GPU skinning: blend position/normal/tangent by joint weights
        mat4 skin_matrix =
            weights.x * transpose(joint_matrices[pc.joint_offset + joints.x].joint) +
            weights.y * transpose(joint_matrices[pc.joint_offset + joints.y].joint) +
            weights.z * transpose(joint_matrices[pc.joint_offset + joints.z].joint) +
            weights.w * transpose(joint_matrices[pc.joint_offset + joints.w].joint);

        skinned_pos = skin_matrix * vec4(position, 1.0);
        skinned_normal = mat3(skin_matrix) * normal;
        skinned_tangent = vec4(mat3(skin_matrix) * tangent.xyz, tangent.w);
    } else {
        skinned_pos = vec4(position, 1.0);
        skinned_normal = normal;
        skinned_tangent = tangent;
    }

    mat4 model_view = transpose(vp.view) * transpose(model);
    gl_Position = transpose(vp.projection) * model_view * skinned_pos;

    vec4 eye_pos = model_view * skinned_pos;
    view_dir = normalize(-eye_pos.xyz);

    // Transform normal to world space using the normal matrix
    mat3 normal_matrix = mat3(transpose(inverse(model)));
    frag_normal = normalize(normal_matrix * skinned_normal);

    // Transform tangent to world space
    vec3 world_tangent = normalize(normal_matrix * skinned_tangent.xyz);
    
    // Calculate bitangent (using tangent.w as the handedness)
    vec3 world_bitangent = normalize(cross(frag_normal, world_tangent) * skinned_tangent.w);
    
    // Re-orthogonalize tangent (Gram-Schmidt process)
    world_tangent = normalize(cross(world_bitangent, frag_normal));

    frag_tangent = world_tangent;
    frag_bitangent = world_bitangent;
    frag_color = color;  // Pass actual vertex colors from glTF
    frag_tex_coord = tex_coord;
}