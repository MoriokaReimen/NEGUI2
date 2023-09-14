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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 out_color;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    out_color = color;
}