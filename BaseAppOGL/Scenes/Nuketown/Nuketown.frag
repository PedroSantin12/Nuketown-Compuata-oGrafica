#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;
in vec3 materialPosition;
in vec3 materialNormal;
out vec4 fragColor;

uniform vec3 uColor;
uniform vec3 uCameraPosition;
uniform vec3 uHazeColor;
uniform vec3 uGarageLampPosition;
uniform vec3 uObjectSize;
uniform int uSurface;
uniform bool uUseAssetMaterials;
uniform bool uHasDiffuseMap;
uniform vec3 uMaterialColor;
uniform sampler2D uDiffuseMap;
uniform sampler2D uGrassMap;
uniform sampler2D uSoilMap;
uniform sampler2D uWoodMap;
uniform sampler2D uRoofMap;

// Smooth deterministic noise adds small wear without another texture library.
float hash(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float noise(vec2 p)
{
    vec2 cell = floor(p);
    vec2 f = fract(p);
    vec2 blend = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(cell), hash(cell + vec2(1, 0)), blend.x),
        mix(hash(cell + vec2(0, 1)), hash(cell + vec2(1, 1)), blend.x), blend.y);
}

float grain(vec2 p)
{
    // Fade detail smaller than a pixel to avoid shimmer while walking.
    float footprint = length(fwidth(p));
    return mix(noise(p), 0.5, smoothstep(0.5, 2.0, footprint));
}

vec2 localUV()
{
    vec3 n = abs(materialNormal);
    if (n.y > n.x && n.y > n.z)
        return materialPosition.xz;
    return n.x > n.z ? materialPosition.zy : materialPosition.xy;
}

vec3 surfaceColor()
{
    vec2 p = localUV();
    vec2 ground = worldPosition.xz;
    if (uSurface == 1) // Asphalt: aggregate, repairs and sparse fine cracks.
    {
        float aggregate = grain(ground * 65.0);
        float patches = noise(ground * 0.43);
        float field = noise(ground * 1.25) + 0.14 * noise(ground * 8.0);
        float contour = abs(field - 0.56);
        float aa = max(fwidth(field), 0.0003);
        float cracks = 1.0 - smoothstep(0.001 - aa, 0.001 + aa, contour);
        cracks *= smoothstep(0.64, 0.80, noise(ground * 0.37 + 19.0));
        return uColor * (0.80 + patches * 0.29 + (aggregate - 0.5) * 0.31)
            * (1.0 - cracks * 0.34);
    }
    if (uSurface == 2) // Concrete slabs in metres, shared by adjacent path pieces.
    {
        vec2 uv = ground / 1.65;
        vec2 edge = min(fract(uv), 1.0 - fract(uv));
        float joint = 1.0 - smoothstep(0.003, 0.009 + max(fwidth(uv.x), fwidth(uv.y)), min(edge.x, edge.y));
        float mottling = noise(ground * 2.3);
        float pores = grain(ground * 90.0);
        return uColor * (0.91 + mottling * 0.15 + (pores - 0.5) * 0.13) * (1.0 - joint * 0.24);
    }
    if (uSurface == 3) // Multi-texture: lawn with irregular dry/earth patches.
    {
        vec3 lawn = texture(uGrassMap, ground * 0.40).rgb;
        vec3 soil = texture(uSoilMap, ground * 0.53).rgb;
        float dry = smoothstep(0.49, 0.79, noise(ground * 0.48)) * 0.40;
        vec3 tint = uColor / vec3(0.37, 0.45, 0.25);
        return mix(lawn * vec3(0.90, 0.94, 0.78), soil * vec3(1.05, 0.89, 0.70), dry)
            * tint * (0.91 + 0.13 * noise(ground * 0.16));
    }
    if (uSurface == 4) // Sand/earth uses the class mud texture, tinted to the site.
    {
        vec3 soil = texture(uSoilMap, ground * 0.46).rgb;
        float value = dot(soil, vec3(0.299, 0.587, 0.114));
        return uColor * (0.62 + value * 1.35) * (0.90 + noise(ground * 0.14) * 0.19);
    }
    if (uSurface == 5) // Painted horizontal boards, constant 19 cm spacing.
    {
        float board = fract((materialPosition.y + uObjectSize.y * 0.5) / 0.19);
        float aa = max(fwidth(board), 0.015);
        float seam = smoothstep(0.0, 0.055 + aa, board);
        float woodgrain = grain(vec2(p.x * 8.0, p.y * 120.0));
        float weather = noise(p * vec2(1.4, 0.7));
        return uColor * (0.80 + seam * 0.19) * (0.94 + 0.10 * weather + (woodgrain - 0.5) * 0.07);
    }
    if (uSurface == 6) // Rotate the supplied board image for vertical fence/door grain.
    {
        vec3 wood = texture(uWoodMap, p.yx * vec2(0.58, 0.48)).rgb;
        float value = dot(wood, vec3(0.299, 0.587, 0.114));
        return uColor * (0.52 + value * 1.65);
    }
    if (uSurface == 7) // Supplied shingle image projected along each roof slope.
    {
        vec3 n = abs(normalize(materialNormal / max(uObjectSize, vec3(0.001))));
        vec2 roofUV = n.x > n.z
            ? vec2(materialPosition.z, materialPosition.y / max(n.x, 0.15))
            : vec2(materialPosition.x, materialPosition.y / max(n.z, 0.15));
        vec3 roof = texture(uRoofMap, roofUV * 0.48).rgb;
        float value = dot(roof, vec3(0.299, 0.587, 0.114));
        return uColor * (0.43 + 1.23 * value);
    }
    if (uSurface == 9) // Painted wood trim keeps its pale finish, with slight wear.
        return uColor * (0.95 + 0.07 * grain(p * vec2(12.0, 80.0)));
    return uColor;
}

void main()
{
    vec3 color = uUseAssetMaterials
        ? (uHasDiffuseMap ? texture(uDiffuseMap, texCoord).rgb : uMaterialColor)
        : surfaceColor();
    vec3 normal = normalize(worldNormal);
    vec3 viewDirection = normalize(uCameraPosition - worldPosition);
    vec3 sunDirection = normalize(vec3(-0.55, 0.85, 0.42));
    float sunlight = max(dot(normal, sunDirection), 0.0);

    vec3 toGarageLamp = uGarageLampPosition - worldPosition;
    float lampDistance = length(toGarageLamp);
    float lampDiffuse = max(dot(normal, toGarageLamp / max(lampDistance, 0.001)), 0.0);
    float lamp = lampDiffuse / (1.0 + 0.5 * lampDistance * lampDistance);
    vec3 shade = color * (vec3(0.53, 0.56, 0.60) + sunlight * vec3(0.51, 0.46, 0.37));
    shade += color * lamp * vec3(1.0, 0.72, 0.40);

    if (!uUseAssetMaterials && uSurface == 8)
    {
        // Opaque tinted glass: sky-colour approximation, not a captured cubemap.
        vec3 reflected = reflect(-viewDirection, normal);
        vec3 skyTint = mix(vec3(0.12, 0.17, 0.16), vec3(0.53, 0.65, 0.69),
            smoothstep(-0.2, 0.65, reflected.y));
        float fresnel = 0.24 + 0.65 * pow(1.0 - max(dot(viewDirection, normal), 0.0), 5.0);
        vec2 p = localUV();
        float streak = grain(vec2(p.x * 70.0, p.y * 1.8));
        shade = mix(color * 0.44, skyTint, fresnel + 0.18) * (0.95 + 0.07 * streak);
        vec3 halfVector = normalize(sunDirection + viewDirection);
        float specular = pow(max(dot(normal, halfVector), 0.0), 110.0);
        shade += vec3(0.95, 0.87, 0.68) * specular * 0.65;
    }

    float haze = smoothstep(28.0, 145.0, distance(uCameraPosition, worldPosition)) * 0.94;
    fragColor = vec4(mix(shade, uHazeColor, haze), 1.0);
}
