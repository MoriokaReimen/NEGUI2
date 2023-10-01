#version 450

layout(location  = 0) in vec4 in_color;

layout(location = 0) out vec4 outColor;
layout(location = 1) out ivec4 outID;

void main() {
    outColor = in_color;
    outID = ivec4(73, 74, 75, 1);
}
