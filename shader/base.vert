#version 450
layout(binding = 0) uniform UniformType
{
    float width;
    float height;
    float x;
    float y;
} ubo;

layout(location = 0) in vec2 inPosition;

void main() {
    float x = (ubo.x - 5000) / 5000;
    float y = (ubo.y - 5000) / 5000;

    gl_Position = vec4(inPosition, 0.0, 1.0)+ vec4(x, y, 0, 0);
}