#version 330

in vec2 fragUV;
in vec3 fragColor;

out vec4 outColor;

uniform sampler2D colorSampler;
uniform sampler2D depthSampler;

void main() {
    vec3 color = texture( colorSampler, fragUV ).rgb;
    float depth = texture( depthSampler, fragUV).r;
    outColor = vec4(color, 1.0);
    gl_FragDepth = depth;
} 