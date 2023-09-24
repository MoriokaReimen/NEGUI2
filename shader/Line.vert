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

layout(location = 0) in vec3 start;
layout(location = 1) in vec3 end;
layout(location = 2) in vec4 color;
layout(location = 3) in float diameter;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 inPosition;
    if(gl_VertexIndex % 2 == 0)
    {
        inPosition = vec4(start, 1.0);
    } else {
        inPosition = vec4(end, 1.0);
    }

    gl_Position = camera.transform * inPosition;
    outColor = color;
}