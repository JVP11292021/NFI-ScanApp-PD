#version 450

layout(location = 0) in vec3 position;

layout(location = 0) out flat uint vObjectID;
layout(location = 1) out flat uint vVertexIndex;
layout(location = 2) out vec3 vWorldPos;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
    uint objectID;
    float pointSize;
} push;


void main() {
    vec4 worldPos = push.model * vec4(position, 1.0);

    gl_Position = ubo.projection * ubo.view * worldPos;
    gl_PointSize = push.pointSize;

    vObjectID     = push.objectID;
    vVertexIndex  = uint(gl_VertexIndex);
    vWorldPos     = worldPos.xyz;
}
