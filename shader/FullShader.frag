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

float arrow(vec3 position, vec3 start, vec3 end, float baseRadius, float tipRadius, float tipHeight)
{
    vec3 t = start - end;
    float l = length(t);
    t /= l;
    l = max(l, tipHeight);

    position -= end;
    if (t.y + 1.0 < 0.0001)
    {
        position.y = -position.y;
    }
    else
    {
        float k = 1.0 / (1.0 + t.y);
        vec3 column1 = vec3(t.z * t.z * k + t.y, t.x, t.z * -t.x * k);
        vec3 column2 = vec3(-t.x, t.y, -t.z);
        vec3 column3 = vec3(-t.x * t.z * k, t.z, t.x * t.x * k + t.y);
        position = mat3(column1, column2, column3) * position;
    }

    vec2 q = vec2(length(position.xz), position.y);
    q.x = abs(q.x);

    // tip
    vec2 e = vec2(tipRadius, tipHeight);
    float h = clamp(dot(q, e) / dot(e, e), 0.0, 1.0);
    vec2 d1 = q - e * h;
    vec2 d2 = q - vec2(tipRadius, tipHeight);
    d2.x -= clamp(d2.x, baseRadius - tipRadius, 0.0);

    // base
    vec2 d3 = q - vec2(baseRadius, tipHeight);
    d3.y -= clamp(d3.y, 0.0, l - tipHeight);
    vec2 d4 = vec2(q.y - l, max(q.x - baseRadius, 0.0));

    float s = max(max(max(d1.x, -d1.y), d4.x), min(d2.y, d3.x));
    return sqrt(min(min(min(dot(d1, d1), dot(d2, d2)), dot(d3, d3)), dot(d4, d4))) * sign(s);
}

mat3 lookAtMatrix(vec3 from, vec3 to)
{
    vec3 forward = normalize(to - from);
    vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, forward);
    return mat3(right, up, forward);
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

vec3 rotate(vec3 v, vec3 axis, float angle)
{
    mat4 m = rotationMatrix(axis, angle);
    return (m * vec4(v, 1.0)).xyz;
}

float sdf(in vec3 position)
{
    //vec3 dir = normalize(vec3(1.0, 0.0, 0.0));
    vec3 dir = normalize(vec3(1.0 * cos(camera.time_ms), 1.0 * sin(camera.time_ms), 0.0));
    
    float angle = atan(dir.y, dir.x);
    position = rotate(position, vec3(0.0, 0.0, -1.0), angle);
    float angleZ = -asin(dir.z);
    position = rotate(position, vec3(0.0, 1.0, 0.0), angleZ);

    float baseRadius = 0.1;
    float tipRadius = 0.3;
    float tipHeight = 0.6;
    float cornerRadius = 0.05;
    vec3 start = vec3(-1.0, 0.0, 0.0);
    vec3 end = vec3(1.0, 0.0, 0.0);
    float d = arrow(position, start, end, baseRadius, tipRadius, tipHeight);
    d -= cornerRadius;
    return d;
}

vec3 normal(vec3 position)
{
    float epsilon = 0.001;
    vec3 gradient = vec3(
        sdf(position + vec3(epsilon, 0, 0)) - sdf(position + vec3(-epsilon, 0, 0)),
        sdf(position + vec3(0, epsilon, 0)) - sdf(position + vec3(0, -epsilon, 0)),
        sdf(position + vec3(0, 0, epsilon)) - sdf(position + vec3(0, 0, -epsilon)));
    return normalize(gradient);
}

float raycast(vec3 rayOrigin, vec3 rayDirection)
{
    int stepCount = 128 * 3;
    float maximumDistance = 10.0;
    float t = 0.0;
    for (int i = 0; i < stepCount; i++)
    {
        if (t > maximumDistance)
        {
            break;
        }
        vec3 currentPosition = rayOrigin + rayDirection * t;
        float d = sdf(currentPosition);
        if (d < 0.0001)
        {
            return t;
        }
        t += d;
    }
    return 0.0;
}

void main()
{
    vec3 targetPosition = vec3(0.0);
    vec3 rayOrigin = vec3(0.0, 0.0, 4.0);
    float timeDelta = 0.25;
    vec3 cameraPosition = vec3(sin(timeDelta * camera.time_ms), 0.0, cos(timeDelta * camera.time_ms));
    
    mat3 eyeTransform = lookAtMatrix(cameraPosition, targetPosition);
    rayOrigin = eyeTransform * rayOrigin;
    
    mat3 cameraTransform = lookAtMatrix(rayOrigin, targetPosition);
    vec3 result = vec3(0.0);
    const float quality = 1.0;
    ivec2 sampleCount = ivec2(quality, quality);
    for (int y = 0; y < sampleCount.y; y++)
    {
        for (int x = 0; x < sampleCount.x; x++)
        {
            vec2 uv = gl_FragCoord.xy + (vec2(float(x), float(y)) / vec2(sampleCount) - 0.5);
            
            uv = uv / camera.resolution.xy;
            uv = (uv * 2.0) - 1.0;
            uv.x *= camera.resolution.x / camera.resolution.y;
            
            vec3 rayDirection = normalize(vec3(uv, 1.5));
            rayDirection = cameraTransform * rayDirection;
            float t = raycast(rayOrigin, rayDirection);
            vec3 color = vec3(0.0);
            if (t > 0.0)
            {
                vec3 position = rayOrigin + rayDirection * t;
                vec3 n = normal(position);
                vec3 lightPosition = 1.0 * normalize(vec3(1.0, 1.0, 1.0));
                
                // diffuse
                vec3 diffuseColor = 1.0 * normalize(vec3(1.0, 0.0, 0.0));
                float diffuseAngle = max(dot(n, lightPosition), 0.0);
                color = diffuseColor * diffuseAngle; // arrow
                
                // ambient
                vec3 ambientColor = 0.1 * normalize(vec3(1, 1, 2));
                color += ambientColor * ((n.y + 1.0) * 0.5);
                
                // specular
                float specularStrength = 30.0;
                vec3 specularColor = ambientColor;//normalize(vec3(1.0, 1.0, 1.0));
                vec3 reflectDir = reflect(lightPosition, n);
                float spec = pow(max(dot(rayDirection, reflectDir), 0.0), 32.0);
                vec3 specular = specularStrength * spec * specularColor;
                color += specular;
            }
            // gamma
            color = sqrt(color);
            result += color;
        }
    }
    result /= float(sampleCount.x * sampleCount.y);
    fragColor = vec4(result, 1.0);
    if(length(result) < 0.01)
    fragColor.a = 0.0;
}