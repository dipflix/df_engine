#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 viewProj;
} ubo;

void main() {
    gl_Position = ubo.viewProj * vec4(inPosition, 1.0);
    fragColor = inColor;
}