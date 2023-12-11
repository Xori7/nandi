#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout( push_constant ) uniform constants {
    mat4 render_matrix;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV0;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV0;

void main() {
    gl_Position = pushConstants.render_matrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragUV0 = inUV0;
}
