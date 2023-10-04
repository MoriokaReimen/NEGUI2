#version 450

layout(location  = 0) in vec4 in_color;
layout(location = 1) in flat int class_id;
layout(location = 2) in flat int instance_id;
layout(location = 3) in flat int vertex_id;

layout(location = 0) out vec4 outColor;
layout(location = 1) out ivec4 outID;

layout(std140, binding = 0) uniform Mouse
{
    float width;
    float height;
    float x;
    float y;
} mouse;

layout(std140, binding = 2) buffer PickData
{
    int type;
    int instance;
    int vertex;
    float depth;
} pick_data;

void main() {
    outColor = in_color;
    outID = ivec4(55, 100, 126, 1);

    /* ピックデータ更新 */
    vec2 pos = vec2(mouse.x, mouse.y);
    float dist = length(gl_FragCoord.xy - pos);
    float depth = 1.0 - gl_FragCoord.z;
    if(dist < 20.0 && pick_data.depth <= depth)
    {
        pick_data.instance = instance_id;
        pick_data.type = class_id;
        pick_data.vertex = vertex_id;
        
        pick_data.depth = depth;
    }
}
