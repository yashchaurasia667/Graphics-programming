#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float Height;
out vec3 Position;

void main() {
  Height = aPos.y;
  Position = (view * model * vec4(aPos, 1.0)).xyz;
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}