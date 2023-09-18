#version 450

layout(std140, binding = 1) uniform Camera
{
   mat4 transform;
   vec2 resolution;
   uint time_ms;
} camera;

layout(origin_upper_left) in vec4 gl_FragCoord;
layout(location = 0) out vec4 fragColor;

/**
 * Convert r, g, b to normalized vec3
 */
vec3 rgb(float r, float g, float b) {
	return vec3(r / 255.0, g / 255.0, b / 255.0);
}

/**
 * Draw a circle at vec2 `pos` with radius `rad` and
 * color `color`.
 */
vec4 circle(vec2 uv, vec2 pos, float rad, vec3 color) {
	float d = rad - length(uv - pos);
	float a = clamp(10 * d, 0.0, 1.0);
    float t = float(d >0.0);
	return vec4(0.0, a, 0.0, t);
}

void main()
{
    vec2 rcp_resolution = 1.0 / camera.resolution.xy;
	vec2 uv = gl_FragCoord.xy * rcp_resolution;
    vec2 ndc = uv;
    vec2 center = vec2(0.5, 0.5);
    vec3 blue = vec3(0.0, 0.0, 1.0);
    vec4 layer = circle(ndc, center, 0.1, blue);

	fragColor = layer;

}