#version 330

uniform mat4 MVP;

in vec2 vPos;
in vec2 vUV;
in vec3 vCol;

out vec3 fragColor;
out vec2 fragUV;

void main() {
    gl_Position = MVP * vec4(vPos,  0.f, 1.0);
    fragUV = vUV;
    fragColor = vCol;
}