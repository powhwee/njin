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
layout (set = 1, binding = 0) uniform sampler2D textures[64];

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

    // Light sources - reduced intensity for better color preservation
    vec3 light_dirs[3];
    light_dirs[0] = normalize(vec3(1.0, 1.0, 1.0));
    light_dirs[1] = normalize(vec3(-1.0, 1.0, -1.0));
    light_dirs[2] = normalize(vec3(1.0, -1.0, -1.0));

    vec3 light_colors[3];
    light_colors[0] = vec3(0.6); // reduced from 1.0
    light_colors[1] = vec3(0.4); // reduced from 0.7
    light_colors[2] = vec3(0.2); // reduced from 0.3

    vec3 total_diffuse = vec3(0.0);
    vec3 total_specular = vec3(0.0);

    for (int i = 0; i < 3; i++) {
        // Diffuse
        float diff = max(dot(N, light_dirs[i]), 0.0);
        total_diffuse += diff * light_colors[i];

        // Specular - reduced
        float specular_strength = 0.3;
        vec3 reflect_dir = reflect(-light_dirs[i], N);
        float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
        total_specular += specular_strength * spec * light_colors[i];
    }

    // Rim lighting - reduced
    float rim_power = 4.0;
    float rim_amount = 1.0 - max(dot(view_dir, N), 0.0);
    float rim = pow(rim_amount, rim_power) * 0.15;
    vec3 rim_color = vec3(1.0, 1.0, 1.0) * rim;

    // Ambient - slightly increased to compensate for reduced direct lighting
    float ambient_strength = 0.25;
    vec3 ambient = ambient_strength * vec3(1.0);

    vec3 result = (ambient + total_diffuse) * base_color + total_specular + rim_color;

    // Lighter tone mapping - preserve more color saturation
    result = result / (result + vec3(0.5));

    // Gamma correction
    float gamma = 2.2;
    result = pow(result, vec3(1.0/gamma));

    out_color = vec4(result, frag_color.a);
}