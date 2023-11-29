#version 330

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform float time;

in vec3 vPos;
in vec2 vUV;
in vec3 vCol;

out vec3 fragColor;
out vec2 fragUV;

void main() {
    vec4 myPos = vec4(vPos, 1.0);
    vec4 timePos =  P *  vec4(0,0,1,1) * 0.3 * time;
    gl_Position = P * V * M * myPos + timePos;
    fragUV = vUV;
    fragColor = vCol;
}