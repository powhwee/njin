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
layout (set = 1, binding = 0) uniform sampler2D textures[16];

// Push constants
layout (push_constant) uniform PushConstants {
    int model_index;
    int texture_index;
} pc;

void main() {
    vec3 N = normalize(frag_normal);
    
    // Determine base color
    vec3 base_color;
    
    if (pc.texture_index >= 0) {
        // Sample texture
        base_color = texture(textures[pc.texture_index], frag_tex_coord).rgb;
    } else {
        // Check if we have non-trivial vertex colors
        bool has_vertex_color = (frag_color.r < 0.99 || frag_color.g < 0.99 || frag_color.b < 0.99)
                             && (frag_color.r > 0.01 || frag_color.g > 0.01 || frag_color.b > 0.01);
        if (has_vertex_color) {
            base_color = frag_color.rgb;
        } else {
            base_color = vec3(0.8);  // Default gray
        }
    }

    // Light sources
    vec3 light_dirs[3];
    light_dirs[0] = normalize(vec3(1.0, 1.0, 1.0));
    light_dirs[1] = normalize(vec3(-1.0, 1.0, -1.0));
    light_dirs[2] = normalize(vec3(1.0, -1.0, -1.0));

    vec3 light_colors[3];
    light_colors[0] = vec3(1.0); // full intensity
    light_colors[1] = vec3(0.7); // slightly less
    light_colors[2] = vec3(0.3); // fill light

    vec3 total_diffuse = vec3(0.0);
    vec3 total_specular = vec3(0.0);

    for (int i = 0; i < 3; i++) {
        // Diffuse
        float diff = max(dot(N, light_dirs[i]), 0.0);
        total_diffuse += diff * light_colors[i];

        // Specular
        float specular_strength = 0.5;
        vec3 reflect_dir = reflect(-light_dirs[i], N);
        float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
        total_specular += specular_strength * spec * light_colors[i];
    }

    // Rim lighting (subtle)
    float rim_power = 4.0;
    float rim_amount = 1.0 - max(dot(view_dir, N), 0.0);
    float rim = pow(rim_amount, rim_power) * 0.3;
    vec3 rim_color = vec3(1.0, 1.0, 1.0) * rim;

    // Ambient
    float ambient_strength = 0.15;
    vec3 ambient = ambient_strength * vec3(1.0);

    vec3 result = (ambient + total_diffuse) * base_color + total_specular + rim_color;

    // Tone mapping (simple Reinhard)
    result = result / (result + vec3(1.0));

    // Gamma correction
    float gamma = 2.2;
    result = pow(result, vec3(1.0/gamma));

    out_color = vec4(result, frag_color.a);
}