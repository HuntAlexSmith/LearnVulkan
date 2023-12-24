#version 450

layout(location = 0) in vec3 fragColor;

// Specify the index of the frame buffer to write to
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
    
}
