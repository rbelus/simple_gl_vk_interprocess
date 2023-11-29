#version 330

uniform mat4 MVP;

// float texture
uniform sampler2D depthSampler;

in vec2 vPos;
in vec2 vUV;
in vec3 vCol;

out vec3 fragColor;
out vec2 fragUV;
out float sampleDepth;

void main() {
    float depth = texture(depthSampler, vUV).r;
    gl_Position = MVP * vec4(vPos, 0.0, depth);
    fragUV = vUV;
    fragColor = vCol;
    sampleDepth = depth;
}