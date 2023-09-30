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
    mat4 model_mat;
} push_constant;

vec3 vertices[24] = vec3[](
    vec3(-0.5, -0.5, -0.5), vec3( 0.5, -0.5, -0.5),
    vec3(-0.5, -0.5, -0.5), vec3(-0.5,  0.5, -0.5),
    vec3( 0.5,  0.5, -0.5), vec3( 0.5, -0.5, -0.5),
    vec3( 0.5,  0.5, -0.5), vec3(-0.5,  0.5, -0.5),
    vec3(-0.5, -0.5, -0.5), vec3(-0.5, -0.5,  0.5),
    vec3( 0.5,  0.5, -0.5), vec3( 0.5,  0.5,  0.5),
    vec3(-0.5,  0.5, -0.5), vec3(-0.5,  0.5,  0.5),
    vec3( 0.5, -0.5, -0.5), vec3( 0.5, -0.5,  0.5),
    vec3(-0.5, -0.5, 0.5), vec3( 0.5, -0.5, 0.5),
    vec3(-0.5, -0.5, 0.5), vec3(-0.5,  0.5, 0.5),
    vec3( 0.5,  0.5, 0.5), vec3( 0.5, -0.5, 0.5),
    vec3( 0.5,  0.5, 0.5), vec3(-0.5,  0.5, 0.5)
);

void main() {
    gl_Position = camera.transform * push_constant.model_mat * vec4(vertices[gl_VertexIndex].xyz, 1.0);
}
