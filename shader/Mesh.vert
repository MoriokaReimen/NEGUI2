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
    int class_id;
    int instance_id;
    mat4 model_mat;
} push_constant;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out int class_id;
layout(location = 2) out int instance_id;
layout(location = 3) out int vertex_id;

void main() {
    gl_Position = camera.transform * push_constant.model_mat * vec4(inPosition, 1.0);
    outNormal = normalize(mat3(inverse(camera.view) * push_constant.model_mat) * inNormal);

    class_id = int(push_constant.class_id);
    instance_id = int(push_constant.instance_id);
    vertex_id = int(gl_VertexIndex);
}