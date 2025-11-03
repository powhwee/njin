#version 450
#extension GL_EXT_debug_printf: enable
#define printf debugPrintfEXT

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec3 frag_normal;
layout (location = 2) in vec3 view_dir;

layout (location = 0) out vec4 out_color;

void main() {
    vec3 normal = normalize(frag_normal);

    // Light sources
    vec3 light_dirs[3];
    light_dirs[0] = normalize(vec3(1.0, 1.0, 1.0));
    light_dirs[1] = normalize(vec3(-1.0, 1.0, -1.0));
    light_dirs[2] = normalize(vec3(1.0, -1.0, -1.0));

    vec3 light_colors[3];
    light_colors[0] = vec3(1.0); // full intensity
    light_colors[1] = vec3(1.0); // full intensity
    light_colors[2] = vec3(0.5); // half intensity

    vec3 total_diffuse = vec3(0.0);
    vec3 total_specular = vec3(0.0);

    for (int i = 0; i < 3; i++) {
        // Diffuse
        float diff = max(dot(normal, light_dirs[i]), 0.0);
        total_diffuse += diff * light_colors[i];

        // Specular
        float specular_strength = 1.0;
        vec3 reflect_dir = reflect(-light_dirs[i], normal);
        float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 64);
        total_specular += specular_strength * spec * light_colors[i];
    }

    // Rim lighting
    float rim_power = 4.0;
    float rim_amount = 1.0 - dot(view_dir, normal);
    float rim = pow(rim_amount, rim_power);
    rim = clamp(rim, 0.0, 1.0);
    vec3 rim_color = vec3(1.0, 1.0, 1.0) * rim;

    // Ambient
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * vec3(1.0);

    vec3 result = (ambient + total_diffuse + total_specular + rim_color) * frag_color.rgb;

    // Exposure control
    float exposure = 0.3;
    result *= exposure;

    // Gamma correction
    float gamma = 2.2;
    result = pow(result, vec3(1.0/gamma));

    out_color = vec4(result, frag_color.a);
}