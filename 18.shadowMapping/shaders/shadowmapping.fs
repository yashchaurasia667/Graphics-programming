#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightingSpace;
in vec2 TexCoords;

uniform sampler2D textureDiffuse;
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

float shadowCalculation(vec4 FragPosLightingSpace, vec3 normal, vec3 lightDir) {
  vec3 projCoords = FragPosLightingSpace.xyz / FragPosLightingSpace.w;
  projCoords = projCoords * 0.5 + 0.5;

  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;

  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    }    
  }
    shadow /= 9.0;

  if(projCoords.z > 1.0) {
    shadow = 0.0;
  }
  return shadow;
}

void main() {
  vec3 color = texture(textureDiffuse, TexCoords).rgb;
  vec3 normal = normalize(Normal);
  vec3 lightColor = vec3(1.0);

  vec3 ambient = 0.15 * color;

  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;

  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
  vec3 specular = spec * lightColor;

  float shadow = shadowCalculation(FragPosLightingSpace, normal, lightDir);

  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
  FragColor = vec4(lighting, 1.0);
}