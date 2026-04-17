#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 TexCoord;

void main()
{
  // convert XYZ vertex to XYZW homogeneous coordinate
  gl_Position = vec4(aPos, 1.0);
  TexCoord = aTex;
}
	