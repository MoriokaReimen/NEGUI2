#version 450

layout(location  = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out ivec4 outID;

void main() {
    outColor = inColor;
    outID = ivec4(0, 0, 0, 1);
}
