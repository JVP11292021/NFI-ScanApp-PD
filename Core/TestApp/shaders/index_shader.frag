#version 450

layout(location = 0) in flat uint vPickID;
layout(location = 1) in vec3 vWorldPos;

layout(location = 0) out uint outPickID;
layout(location = 1) out vec4 outWorldPos;

void main() {
    outPickID = vPickID;
    outWorldPos = vec4(vWorldPos, 1.0);
}