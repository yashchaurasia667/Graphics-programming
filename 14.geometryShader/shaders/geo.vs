#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
  vec2 texCoords;
  vec3 normal;
} vs_out;

void main()
{
  gl_Position = view * model * vec4(aPos.x, aPos.y, 0.0, 1.0); 
  mat3 normalMatrix = mat3(transpose(inverse(view * model)));
  vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
  vs_out.texCoords = aTexCoords;
}