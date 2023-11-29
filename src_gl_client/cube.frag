#version 330

in vec2 fragUV;
in vec3 fragColor;

out vec4 outColor;

void main() { 
    outColor = vec4(fragColor * gl_FragCoord.z, 1.0);
} 