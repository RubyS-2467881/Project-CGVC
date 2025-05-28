#version 330 core
#define MAX_LIGHTS 100

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = material.ambient * 0.2;
    vec3 diffuse = material.diffuse * diff;
    vec3 specular = material.specular * spec;

    vec3 color = (ambient + diffuse + specular) * lightColor;
    FragColor = vec4(color, 1.0);
} 