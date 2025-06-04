#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D chromaTexture;
uniform sampler2D newTexture;
uniform vec3 keyColorYCbCr;       // chroma key in YCbCr
uniform float threshold;          // how close to keyColor to discard

vec3 rgb2ycbcr(vec3 color) {
    float Y  =  0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
    float Cb = -0.169 * color.r - 0.331 * color.g + 0.5   * color.b + 0.5;
    float Cr =  0.5   * color.r - 0.419 * color.g - 0.081 * color.b + 0.5;
    return vec3(Y, Cb, Cr);
}

void main() {
    vec3 texColor = texture(chromaTexture, TexCoords).rgb;
    vec3 texYCbCr = rgb2ycbcr(texColor);

    float distanceToKey = distance(texYCbCr.yz, keyColorYCbCr.yz); // only chroma channels
    if (distanceToKey < threshold)
        FragColor = texture(newTexture, TexCoords);
    else
        FragColor = vec4(texColor, 1.0);
}
