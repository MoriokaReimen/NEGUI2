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

// Grid position are in xy clipped space
vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;
layout(location = 3) out mat4 pv;

vec3 UnprojectPoint(float x, float y, float z, mat4 transform) {
    mat4 transform_inv = inverse(transform);
    vec4 unprojectedPoint =  transform_inv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

// normal vertice projection
void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    pv = camera.projection * inverse(camera.view);
    nearPoint = UnprojectPoint(p.x, p.y, 0.0, pv).xyz; // unprojecting on the near plane
    farPoint = UnprojectPoint(p.x, p.y, 1.0, pv).xyz; // unprojecting on the far plane
    gl_Position = vec4(p, 1.0); // using directly the clipped coordinates

}

