#version 450

// Note: dvec usues two layout location slots
// Example:
// layout(location = 0) in dvec3 inPosition;
// layout(location = 2) in vec3 inColor;
// Input / Attribute variables
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor
}
