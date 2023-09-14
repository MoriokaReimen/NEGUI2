#version 450
layout(binding = 0) uniform Mouse
{
    float width;
    float height;
    float x;
    float y;
} mouse;

layout(binding = 1) uniform Camera
{
   mat4 transform; 
} camera;

layout(location = 0) in vec2 inPosition;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
}