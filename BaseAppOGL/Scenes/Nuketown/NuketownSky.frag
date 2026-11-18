#version 330 core
in vec3 viewRay;
out vec4 fragColor;
uniform vec3 uSunDirection;
uniform bool uBakeSky;
uniform samplerCube uEnvironmentMap;
const vec3 sunColour = vec3(1.0, 0.96, 0.80);

float SunDisc(vec3 ray)
{
    float alignment = dot(ray, normalize(uSunDirection));
    float radius = cos(radians(0.75));
    float antialias = max(fwidth(alignment), 0.0000005);
    return smoothstep(radius - antialias, radius + antialias, alignment);
}

float hash3(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.yxz + 33.33);
    return fract((p.x + p.y) * p.z);
}

float SkyNoise3(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(hash3(i), hash3(i + vec3(1, 0, 0)), f.x),
                   mix(hash3(i + vec3(0, 1, 0)), hash3(i + vec3(1, 1, 0)), f.x), f.y),
               mix(mix(hash3(i + vec3(0, 0, 1)), hash3(i + vec3(1, 0, 1)), f.x),
                   mix(hash3(i + vec3(0, 1, 1)), hash3(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}

vec3 SkyColor(vec3 direction)
{
    vec3 ray = normalize(direction);
    float elevation = clamp(ray.y, 0.0, 1.0);
    vec3 sky = mix(vec3(0.76, 0.81, 0.82), vec3(0.24, 0.43, 0.66), pow(elevation, 0.45));
    vec3 cloudPoint = ray * 4.0 + vec3(1.7, 0.8, 2.6);
    float cloud = SkyNoise3(cloudPoint) * 0.60
        + SkyNoise3(cloudPoint * 2.2 + 5.7) * 0.28
        + SkyNoise3(cloudPoint * 4.8 + 11.3) * 0.12;
    float coverage = smoothstep(0.51, 0.72, cloud) * smoothstep(0.025, 0.22, ray.y);
    sky = mix(sky, vec3(0.93, 0.94, 0.92), coverage * 0.87);
    float sun = max(dot(ray, normalize(uSunDirection)), 0.0);
    float halo = exp(-(1.0 - sun) * 1600.0) * 0.12;
    sky += vec3(1.0, 0.88, 0.65) * halo;
    // A continuous fog-coloured horizon hides the distant terrain boundary.
    if (ray.y < 0.0)
        return mix(vec3(0.76, 0.81, 0.82), vec3(0.59, 0.54, 0.44),
            smoothstep(0.0, 0.35, -ray.y));
    return sky;
}
void main()
{
    vec3 ray = normalize(viewRay);
    float disc = SunDisc(ray);
    if (uBakeSky)
    {
        // RGB keeps the sun for vehicle/window reflections. Alpha records its
        // coverage so the visible sky can replace that low-resolution disc.
        fragColor = vec4(mix(SkyColor(ray), sunColour, disc), disc);
    }
    else
    {
        vec4 environment = texture(uEnvironmentMap, ray);
        vec3 sky = clamp((environment.rgb - environment.a * sunColour)
            / max(1.0 - environment.a, 0.0001), vec3(0.0), vec3(1.0));
        if (environment.a > 0.75)
        {
            // Near an opaque cached disc, avoid amplifying RGBA8 rounding.
            vec3 sunDirection = normalize(uSunDirection);
            vec3 reference = abs(sunDirection.y) > 0.95 ? vec3(1, 0, 0) : vec3(0, 1, 0);
            vec3 tangent = normalize(cross(sunDirection, reference));
            vec3 nearbySky = texture(uEnvironmentMap, normalize(sunDirection + tangent * 0.04)).rgb;
            sky = mix(sky, nearbySky, smoothstep(0.75, 0.95, environment.a));
        }
        // The camera-facing disc is evaluated at screen resolution, while the
        // clouds and halo stay cached. It replaces rather than adds a second sun.
        fragColor = vec4(mix(sky, sunColour, disc), 1.0);
    }
}
