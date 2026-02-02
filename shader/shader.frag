#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 frag_normal;
layout (location = 2) in vec3 frag_tangent;
layout (location = 3) in vec3 frag_bitangent;
layout (location = 4) in vec3 view_dir;
layout (location = 5) in vec2 frag_tex_coord;

layout (location = 0) out vec4 out_color;

// Texture samplers (set 1, binding 0)
layout (set = 1, binding = 0) uniform sampler2D textures[1024];

// Push constants
layout (push_constant) uniform PushConstants {
    int model_index;
    int texture_index;
    float base_color_r;  // Individual floats to match C++ struct layout
    float base_color_g;
    float base_color_b;
    float base_color_a;
} pc;

void main() {
    vec3 N = normalize(frag_normal);
    
    // Determine base color
    vec3 base_color;
    
    if (pc.texture_index >= 0) {
        // Sample texture and multiply by material color
        vec3 mat_color = vec3(pc.base_color_r, pc.base_color_g, pc.base_color_b);
        base_color = texture(textures[pc.texture_index], frag_tex_coord).rgb * mat_color;
    } else {
        // Use material's base color factor
        base_color = vec3(pc.base_color_r, pc.base_color_g, pc.base_color_b);
    }

    // Multiple directional lights
    vec3 light_dirs[3];
    light_dirs[0] = normalize(vec3(1.0, 1.0, 1.0));
    light_dirs[1] = normalize(vec3(-1.0, 0.5, -0.5));
    light_dirs[2] = normalize(vec3(0.0, -1.0, 0.5));

    vec3 light_colors[3];
    light_colors[0] = vec3(0.6);  // Main light
    light_colors[1] = vec3(0.3);  // Fill light
    light_colors[2] = vec3(0.15); // Back light

    vec3 total_diffuse = vec3(0.0);
    vec3 total_specular = vec3(0.0);

    for (int i = 0; i < 3; i++) {
        // Diffuse
        float diff = max(dot(N, light_dirs[i]), 0.0);
        total_diffuse += diff * light_colors[i];

        // Specular (Blinn-Phong)
        vec3 halfway = normalize(light_dirs[i] + view_dir);
        float spec = pow(max(dot(N, halfway), 0.0), 32.0);
        total_specular += spec * light_colors[i] * 0.2;
    }



    // Ambient
    float ambient = 0.2;

    // Combine (specular OK, rim causes issues)
    vec3 result = (ambient + total_diffuse) * base_color + total_specular;

    out_color = vec4(result, frag_color.a);
}