#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform vec2 texelSize;
uniform int kernelType;

vec2 offsets[9] = vec2[](
    vec2(-texelSize.x,  texelSize.y),
    vec2( 0.0f,          texelSize.y),
    vec2( texelSize.x,  texelSize.y),
    vec2(-texelSize.x,  0.0f),
    vec2( 0.0f,          0.0f),
    vec2( texelSize.x,  0.0f),
    vec2(-texelSize.x, -texelSize.y),
    vec2( 0.0f,         -texelSize.y),
    vec2( texelSize.x, -texelSize.y)
);

vec4 applyConvolution(float kernel[9]) {
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
        sampleTex[i] = vec3(texture(screenTexture, TexCoords + offsets[i]));

    vec3 color = vec3(0.0);
    for(int i = 0; i < 9; i++)
        color += sampleTex[i] * kernel[i];

    color = clamp(color, 0.0, 1.0);
    return vec4(color, 1.0);
}

void main() {
    float identityKernel[9] = float[](
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    );

    float edgeKernel[9] = float[](
        1,  1,  1,
        1, -8,  1,
        1,  1,  1
    );

    float blurKernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );

    float sharpenKernel[9] = float[](
         0, -1,  0,
        -1,  5, -1,
         0, -1,  0
    );

    if (kernelType == 1)
        FragColor = applyConvolution(edgeKernel);
    else if (kernelType == 2)
        FragColor = applyConvolution(blurKernel);
    else if (kernelType == 3)
        FragColor = applyConvolution(sharpenKernel);
    else 
        FragColor = applyConvolution(identityKernel);
}
