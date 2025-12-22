#version 450

layout(location = 0) in vec3 position;

layout(location = 0) out flat uint vPickID;
layout(location = 1) out vec3 vWorldPos;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint objectID;
    float pointSize;
} push;

void main() {
    vec4 worldPos = push.modelMatrix * vec4(position, 1.0);
    vec4 viewPos = ubo.view * worldPos;

    gl_Position = ubo.projection * viewPos;
    gl_PointSize = push.pointSize;

    vPickID = (push.objectID << 16) | (gl_VertexIndex & 0xFFFF);
    vWorldPos = worldPos.xyz;
}