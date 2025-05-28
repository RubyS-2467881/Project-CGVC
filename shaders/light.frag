#version 330 core
#define MAX_LIGHTS 100

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform PointLight lights[MAX_LIGHTS];
uniform int numLights;

uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    for (int i = 0; i < numLights; ++i) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);

        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (distance * distance + 1.0);

        vec3 ambient = 0.05 * lights[i].color;
        vec3 diffuse = diff * lights[i].color;
        vec3 specular = spec * lights[i].color;

        vec3 contribution = (ambient + diffuse + specular) * attenuation * lights[i].intensity;

        result += contribution;
    }

    FragColor = vec4(result, 1.0);
}
