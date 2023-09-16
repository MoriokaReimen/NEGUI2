#version 450
layout(std140, binding = 0) uniform Mouse
{
    float width;
    float height;
    float x;
    float y;
} mouse;

layout(std140, binding = 1) uniform Camera
{
   mat4 transform; 
} camera;

layout (push_constant) uniform PushBlock
{
    uint class_id;
    uint instance_id;
    mat4 model_mat;
} push_constant;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 out_color;

void main() {
    gl_Position = camera.transform * push_constant.model_mat * vec4(inPosition, 1.0);
    out_color = color;
}