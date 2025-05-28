#version 330 core
#define MAX_POINT_LIGHTS 16

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform int numPointLights;
uniform PointLight pointLights[MAX_POINT_LIGHTS];

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform sampler2D texture1;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 texColor = texture(texture1, TexCoords).rgb;
    vec3 result = texColor * 0.2; // ambient

    for (int i = 0; i < numPointLights; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float distance = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.05 * distance * distance); // basic attenuation

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = texColor * diff * pointLights[i].color * pointLights[i].intensity;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = vec3(0.3) * spec * pointLights[i].color * pointLights[i].intensity;

        result += attenuation * (diffuse + specular);
    }

    FragColor = vec4(result, 1.0);
}