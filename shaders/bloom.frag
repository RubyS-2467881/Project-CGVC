#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;
uniform float intensity;

void main() {
    vec3 base = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloom, TexCoords).rgb;
    FragColor = vec4(base + bloomColor * intensity, 1.0);
}