// DATA ALIGNMENT:
//  Scalars must be aligned by N (4 bytes given 32 bit floats)
//  vec2 must be aligned by 2N (8 bytes)
//  vec3 or vec4 must be aligned by 4N (16 bytes)
//  nested structure bust be aligned by base alignment of its members
//      rounded up to a multiple of 16
//  mat4 matrix must have same alignment as a vec4
// Alignment specifications can be found here:
//  https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
#version 450

// Uniform block
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Note: dvec usues two layout location slots
// Example:
// layout(location = 0) in dvec3 inPosition;
// layout(location = 2) in vec3 inColor;
// Input / Attribute variables
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
