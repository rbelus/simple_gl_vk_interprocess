#version 330

in vec2 fragUV;
in vec3 fragColor;

out vec4 outColor;

uniform sampler2D myTextureSampler;

void main() {
    vec3 color = texture( myTextureSampler, fragUV ).rgb;
    outColor = vec4(color, 1.0);
} 