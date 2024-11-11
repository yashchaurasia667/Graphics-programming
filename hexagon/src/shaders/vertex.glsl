#version 330 core

layout (location=0) in vec3 Pos;
layout (location=1) in int colorIndex;

out vec3 fragmentColor;

const vec3 colors[4] = vec3[] (
  vec3(1.0, 0.0, 0.0),
  vec3(0.0, 1.0, 0.0),
  vec3(0.0, 0.0, 1.0),
  vec3(0.3, 0.4, 0.5)
);

void main() {
  gl_Position = vec4(Pos, 1.0);
  fragmentColor = colors[colorIndex];
}