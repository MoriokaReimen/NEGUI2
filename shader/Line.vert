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
layout(location = 1) out vec3 outPosition;
layout(location = 2) out vec3 outStart;
layout(location = 3) out vec3 outEnd;

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
    vec4 inPosition;
    vec3 dir = end - start;
    mat4 inv = inverse(camera.transform);
    vec3 up = normalize(cross(dir, inv[2].xyz));

    if(gl_VertexIndex % 4 == 1)
    {
        inPosition = vec4(start - 0.5 * diameter * up, 1.0);
    } else if(gl_VertexIndex % 4 == 2) {
        inPosition = vec4(end - 0.5 * diameter * up, 1.0);
    } else if(gl_VertexIndex % 4 == 3) {
        inPosition = vec4(end + 0.5 * diameter * up, 1.0);
    } else if(gl_VertexIndex % 4 == 0) {
        inPosition = vec4(start + 0.5 * diameter * up, 1.0);
    }

    gl_Position = camera.transform * inPosition;
    outColor = color;
    outPosition = inPosition.xyz / inPosition.w;
    outStart = start;
    outEnd = end;
}