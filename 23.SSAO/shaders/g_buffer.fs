#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 Normal;
in vec3 Fragpos;
in vec2 TexCoords;

void main() {
  gPosition = Fragpos;
  gNormal = normalize(Normal);
  gAlbedoSpec.rgb = vec3(0.95);
}