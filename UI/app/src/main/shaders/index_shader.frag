#version 450

layout(location = 0) in flat uint vObjectID;
layout(location = 1) in flat uint vVertexIndex;
layout(location = 2) in vec3 vWorldPos;

layout(location = 0) out uvec2 outPick;
layout(location = 1) out vec4  outWorldPos;

void main() {
    outPick = uvec2(vObjectID, vVertexIndex);
    outWorldPos = vec4(vWorldPos, 1.0);
}
