#version 450

layout(location  = 0) in vec4 inColor;
layout(location = 1) in vec3 position;
layout(location = 2) in vec3 inStart;
layout(location = 3) in vec3 inEnd;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outID;

void main() {
    vec3 direction = normalize(inEnd - inStart);
    vec3 diff = position - inStart;
    float leg = dot(diff, direction);
    float rad = length(diff -leg * direction);
    outColor = (1.0 - rad) * inColor;
    outID = vec4(1.0, 0.0, 0.0, 1.0);
}
