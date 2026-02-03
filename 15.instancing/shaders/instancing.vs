#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;

uniform float time;

out vec3 fColor;

void main() {
  bool flip = false;
  vec2 pos = aPos * sin(time);
  if(sin(time) == 0)
    flip = !flip;
  gl_Position = vec4(pos + aOffset, -0.5, 1.0);

  if(!flip)
    fColor = aColor;
  else 
    fColor = 1-aColor;
}