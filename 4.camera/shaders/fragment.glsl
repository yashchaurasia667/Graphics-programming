#version 330 core 

in vec2 TexCoords;
uniform sampler2D aTex;

out vec4 FragColor;

void main() {
  FragColor = texture(aTex, TexCoords);
}