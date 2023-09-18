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
   vec2 resolution;
   uint time_ms;
} camera;

layout (push_constant) uniform PushBlock
{
    uint class_id;
    uint instance_id;
    mat4 model_mat;
} push_constant;

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

layout(location = 0) out mat4 pv;
layout(location = 4) out vec2 resolution;
layout(location = 5) out uint time_ms;

// normal vertice projection
void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    gl_Position = vec4(p.xy, 0.1, 1.0); // using directly the clipped coordinates
    pv = camera.transform;
    resolution = camera.resolution;
    time_ms = camera.time_ms;
}

