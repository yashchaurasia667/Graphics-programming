#version 450

// Hardcoded NDC positions for a full-screen-ish triangle
vec2 positions[3] = vec2[](
    vec2( 0.0, -0.6),
    vec2( 0.6,  0.6),
    vec2(-0.6,  0.6)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.2, 0.2),  // red   - top
    vec3(0.2, 1.0, 0.4),  // green - bottom right
    vec3(0.3, 0.5, 1.0)   // blue  - bottom left
);

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor   = colors[gl_VertexIndex];
}
