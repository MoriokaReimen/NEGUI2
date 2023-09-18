#version 450

layout(location = 0) in mat4 pv;
layout(location = 4) in vec2 resolution;
layout(location = 5) in flat uint time_ms;

layout(location = 0) out vec4 fragColor;

/**
 * @author jonobr1 / http://jonobr1.com/
 */

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
	float d = length(pos - uv) - rad;
	float t = clamp(d, 0.0, 1.0);
	return vec4(color, 1.0 - t);
}

void main()
{
	vec2 uv = gl_FragCoord.xy / 1920.0;
	float blue = float(time_ms % 10000) / 10000.0;
	// Blend the two
	fragColor = vec4(0.0, 0.0, blue, 0.5);

}