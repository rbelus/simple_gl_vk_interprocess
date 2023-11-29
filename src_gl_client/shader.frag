#version 330

in vec2 fragUV;
in vec3 fragColor;
in float sampleDepth;

out vec4 outColor;

uniform sampler2D colorSampler;
uniform sampler2D depthSampler;

void main() {
    float depth = texture(depthSampler, fragUV).r;
    if (depth < .8f)
    {
        discard;
    }
    vec3 color = texture( colorSampler, fragUV ).rgb;
    outColor = vec4(color, 1.0);
} 