#version 330 core

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float cutOff;
  float outerCutOff;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 cubeColor;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform Material material;
uniform Light light;

out vec4 FragColor;

void main() {
  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(light.position - FragPos);
  float diff= max(dot(norm, lightDir), 0);

  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec= pow(max(dot(viewDir, reflectDir), 0), material.shininess);

  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);  
  vec3 result = vec3(0.0);
    
  if(theta > light.cutOff) 
  {       
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    diffuse *= intensity;
    specular *= intensity;

    result = ambient + diffuse + specular; 
  }
  else  
    result = light.ambient * vec3(texture(material.diffuse, TexCoords));
  FragColor = vec4(result, 1.0f);
}