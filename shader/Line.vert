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
   mat4 projection;
   mat4 view;
   vec2 resolution;
} camera;

layout (push_constant) uniform PushBlock
{
    uint class_id;
    uint instance_id;
    mat4 model_mat;
} push_constant;

layout(location = 0) in vec3 start;
layout(location = 1) in vec3 end;
layout(location = 2) in vec4 color;
layout(location = 3) in float diameter;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 inPosition = start;
    if(gl_VertexIndex % 2 == 1)
    {
        inPosition = end;
    }

    gl_Position = camera.transform * vec4(inPosition, 1.0);
    outColor = color;
}