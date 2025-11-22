#version 330 core

in vec3 colr;

out vec4 FragColor;

void main() {
  FragColor = vec4(colr , 1.0f);
}