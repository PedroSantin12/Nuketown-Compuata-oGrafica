#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 texCoord;
in vec3 materialPosition;
in vec3 materialNormal;
in vec4 lightSpacePosition;
out vec4 fragColor;

uniform vec3 uColor;
uniform vec3 uCameraPosition;
uniform vec3 uHazeColor;
uniform vec3 uGarageLampPosition;
uniform vec3 uSecondGarageLampPosition;
uniform vec3 uObjectSize;
uniform vec3 uSunDirection;
uniform int uStreetLightCount;
uniform vec3 uStreetLightPositions[8];
uniform float uTime;
uniform int uSurface;
uniform bool uUseAssetMaterials;
uniform bool uHasDiffuseMap;
uniform vec3 uMaterialColor;
uniform sampler2D uDiffuseMap;
uniform sampler2D uGrassMap;
uniform sampler2D uSoilMap;
uniform sampler2D uWoodMap;
uniform sampler2D uRoofMap;
uniform sampler2D uSignMap;
uniform sampler2D uPoliceMap;
uniform sampler2D uDeliveryMap;
uniform sampler2D uStoneMap;
uniform sampler2D uShadowMap;
uniform sampler2D uStreetLightMap;
uniform samplerCube uEnvironmentMap;

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
        float aggregate = grain(ground * 105.0);
        float patches = noise(ground * 0.43);
        float field = noise(ground * 1.25) + 0.14 * noise(ground * 8.0);
        float contour = abs(field - 0.56);
        float aa = max(fwidth(field), 0.0003);
        float cracks = 1.0 - smoothstep(0.001 - aa, 0.001 + aa, contour);
        cracks *= smoothstep(0.64, 0.80, noise(ground * 0.37 + 19.0));
        return uColor * (0.89 + patches * 0.15 + (aggregate - 0.5) * 0.22)
            * (1.0 - cracks * 0.34);
    }
    if (uSurface == 2) // Concrete slabs in metres, shared by adjacent path pieces.
    {
        vec2 uv = ground / 1.65;
        vec2 edge = min(fract(uv), 1.0 - fract(uv));
        float joint = 1.0 - smoothstep(0.003, 0.009 + max(fwidth(uv.x), fwidth(uv.y)), min(edge.x, edge.y));
        float mottling = noise(ground * 3.8);
        float pores = grain(ground * 135.0);
        return uColor * (0.94 + mottling * 0.09 + (pores - 0.5) * 0.10) * (1.0 - joint * 0.22);
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
    if (uSurface == 5 || uSurface == 14) // Horizontal or vertical painted siding.
    {
        float axis = uSurface == 14 ? p.x : materialPosition.y + uObjectSize.y * 0.5;
        float board = fract(axis / 0.19);
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
        if (abs(materialNormal.y) > 0.95)
            roofUV = materialPosition.zx;
        vec3 roof = texture(uRoofMap, roofUV * 0.48).rgb;
        float value = dot(roof, vec3(0.299, 0.587, 0.114));
        return uColor * (0.43 + 1.23 * value);
    }
    if (uSurface == 9) // Painted wood trim keeps its pale finish, with slight wear.
        return uColor * (0.95 + 0.07 * grain(p * vec2(12.0, 80.0)));
    if (uSurface == 10)
        return texture(uSignMap, texCoord).rgb;
    if (uSurface == 11)
        return texture(uPoliceMap, texCoord).rgb;
    if (uSurface == 12)
        return texture(uDeliveryMap, texCoord).rgb;
    if (uSurface == 13)
    {
        vec3 stone = texture(uStoneMap, p * 0.72).rgb;
        return uColor * (0.48 + 1.35 * dot(stone, vec3(0.299, 0.587, 0.114)));
    }
    if (uSurface == 19)
        return uColor * (0.80 + 0.20 * grain(p * 105.0));
    if (uSurface == 21) // Warm desert sand with restrained, non-repeating variation.
    {
        float dune = noise(ground * 0.022) * 0.65 + noise(ground * 0.071 + 17.0) * 0.35;
        float sandGrain = grain(ground * 34.0);
        vec3 soil = texture(uSoilMap, ground * 0.19).rgb;
        float soilValue = dot(soil, vec3(0.299, 0.587, 0.114));
        return uColor * (0.88 + dune * 0.19 + (soilValue - 0.37) * 0.13
            + (sandGrain - 0.5) * 0.07);
    }
    if (uSurface == 22)
        return uColor;
    if (uSurface == 23) // Rocky slopes and sandy shelves remain matte.
    {
        vec3 rock = texture(uStoneMap, ground * 0.045).rgb;
        vec3 soil = texture(uSoilMap, ground * 0.085).rgb;
        float rockValue = dot(rock, vec3(0.299, 0.587, 0.114));
        float soilValue = dot(soil, vec3(0.299, 0.587, 0.114));
        float slope = 1.0 - max(normalize(worldNormal).y, 0.0);
        float exposed = smoothstep(0.08, 0.48, slope)
            * smoothstep(1.0, 8.0, worldPosition.y);
        vec3 sandy = uColor * vec3(1.08, 1.04, 0.94) * (0.86 + soilValue * 0.42);
        vec3 rocky = uColor * vec3(0.94, 0.97, 1.02) * (0.77 + rockValue * 0.55);
        return mix(sandy, rocky, exposed) * (0.97 + 0.06 * noise(ground * 0.17));
    }
    if (uSurface == 24) // Fronds and sparse desert shrubs.
    {
        float leaf = noise(p * vec2(11.0, 3.0));
        return uColor * (0.88 + leaf * 0.19);
    }
    return uColor;
}

// The same procedural daylight can be used by the background sky shader.
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
    return textureLod(uEnvironmentMap, normalize(direction), 0.5).rgb;
}

float ShadowFactor(vec3 geometricNormal, vec3 sunDirection)
{
    if (lightSpacePosition.w <= 0.0)
        return 0.0;
    vec3 projected = lightSpacePosition.xyz / lightSpacePosition.w;
    projected = projected * 0.5 + 0.5;
    if (projected.x < 0.0 || projected.x > 1.0 || projected.y < 0.0 || projected.y > 1.0
        || projected.z < 0.0 || projected.z > 1.0)
        return 0.0;
    float bias = max(0.0009 * (1.0 - max(dot(geometricNormal, sunDirection), 0.0)), 0.00028);
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
        {
            float nearestDepth = texture(uShadowMap, projected.xy + vec2(x, y) * texel).r;
            shadow += projected.z - bias > nearestDepth ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}

vec3 MicroNormal(vec3 normal)
{
    float height = 0.0;
    if (uSurface == 1)
        height = (grain(worldPosition.xz * 105.0) - 0.5) * 0.0013;
    else if (uSurface == 2)
        height = (grain(worldPosition.xz * 135.0) - 0.5) * 0.0008;
    else if (uSurface == 13)
        height = noise(localUV() * 18.0) * 0.003;
    else if (uSurface == 21)
        height = (grain(worldPosition.xz * 34.0) - 0.5) * 0.0008;
    else
        return normal;

    vec3 dx = dFdx(worldPosition);
    vec3 dy = dFdy(worldPosition);
    vec3 r1 = cross(dy, normal);
    vec3 r2 = cross(normal, dx);
    float determinant = dot(dx, r1);
    if (abs(determinant) < 0.0000001)
        return normal;
    vec3 gradient = sign(determinant) * (dFdx(height) * r1 + dFdy(height) * r2);
    return normalize(abs(determinant) * normal - gradient);
}

float LampDiffuse(vec3 lampPosition, vec3 normal)
{
    vec3 span = lampPosition - worldPosition;
    float distanceToLamp = length(span);
    return max(dot(normal, span / max(distanceToLamp, 0.001)), 0.0)
        / (1.0 + 0.5 * distanceToLamp * distanceToLamp);
}


// The down-facing fixtures are bounded before normalization and shading work.
vec3 StreetLighting(vec3 normal)
{
    // Static ground lighting uses one lookup instead of evaluating eight fixtures
    // for every visible desert/road fragment. Upright surfaces retain the exact spots.
    if (normal.y > 0.8 && worldPosition.y < 0.35)
    {
        vec2 uv = (worldPosition.xz - vec2(-52.0, -35.0)) / vec2(106.0, 72.0);
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            return vec3(0.0);
        return texture(uStreetLightMap, uv).rgb;
    }
    vec3 lighting = vec3(0.0);
    int count = clamp(uStreetLightCount, 0, 8);
    for (int index = 0; index < count; ++index)
    {
        vec3 toLight = uStreetLightPositions[index] - worldPosition;
        float distanceSquared = dot(toLight, toLight);
        float radius = index < 4 ? 8.0 : 13.0;
        if (distanceSquared >= radius * radius || toLight.y <= 0.0)
            continue;
        float inverseDistance = inversesqrt(max(distanceSquared, 0.0001));
        float cone = smoothstep(0.422618, 0.939693, toLight.y * inverseDistance);
        if (cone <= 0.0)
            continue;
        float diffuse = max(dot(normal, toLight * inverseDistance), 0.0);
        float distanceToLight = distanceSquared * inverseDistance;
        float falloff = max(0.0, 1.0 - distanceSquared / (radius * radius));
        float attenuation = falloff * falloff
            / (1.0 + 0.15 * distanceToLight + 0.06 * distanceSquared);
        float intensity = index < 4 ? 1.8 : 3.0;
        lighting += vec3(1.0, 0.81, 0.53) * diffuse * cone * attenuation * intensity;
    }
    return lighting;
}

void main()
{
    if (!uUseAssetMaterials && uSurface == 20)
    {
        vec3 emission = mix(uColor, vec3(1.0, 0.86, 0.54), 0.80) * 1.08;
        fragColor = vec4(min(emission, vec3(1.0)), 1.0);
        return;
    }
    if (!uUseAssetMaterials && uSurface == 15)
    {
        float radius = length(texCoord * 2.0 - 1.0);
        float alpha = (1.0 - smoothstep(0.24, 1.0, radius)) * 0.22;
        fragColor = vec4(0.04, 0.05, 0.045, alpha);
        return;
    }

    vec3 color = uUseAssetMaterials
        ? (uHasDiffuseMap ? texture(uDiffuseMap, texCoord).rgb : uMaterialColor)
        : surfaceColor();
    vec3 geometricNormal = normalize(worldNormal);
    vec3 normal = uUseAssetMaterials ? geometricNormal : MicroNormal(geometricNormal);
    vec3 viewDirection = normalize(uCameraPosition - worldPosition);
    vec3 sunDirection = normalize(uSunDirection);
    float sunlight = max(dot(normal, sunDirection), 0.0);
    float visibility = 1.0 - ShadowFactor(geometricNormal, sunDirection);

    // Cool, open-sky fill retains detail in the shadows; only sunlight is occluded.
    float skyFill = 0.37 + max(normal.y, 0.0) * 0.09;
    vec3 ambient = vec3(0.92, 0.98, 1.05) * skyFill;
    vec3 direct = sunlight * visibility * vec3(0.72, 0.70, 0.65);
    if (!uUseAssetMaterials && uSurface >= 10 && uSurface <= 12)
        ambient = max(ambient, vec3(0.48));
    vec3 shade = color * (ambient + direct);
    float lamp = LampDiffuse(uGarageLampPosition, normal)
        + LampDiffuse(uSecondGarageLampPosition, normal);
    shade += color * lamp * vec3(0.85, 0.65, 0.40);
    shade += color * StreetLighting(normal);

    if (!uUseAssetMaterials)
    {
        vec3 halfVector = normalize(sunDirection + viewDirection);
        float nDotV = max(dot(normal, viewDirection), 0.0);
        float nDotH = max(dot(normal, halfVector), 0.0);
        if (uSurface == 8 || uSurface == 18)
        {
            vec3 reflected = reflect(-viewDirection, normal);
            vec3 environment = SkyColor(reflected);
            bool vehicle = uSurface == 18;
            vec3 glassInterior = vehicle ? vec3(0.045, 0.082, 0.095) : color * 0.28;
            float fresnel = pow(1.0 - nDotV, 5.0);
            float reflection = (vehicle ? 0.40 : 0.34) + fresnel * 0.48;
            shade = mix(glassInterior * (0.7 + 0.3 * visibility), environment, reflection);
            float edge = min(min(texCoord.x, 1.0 - texCoord.x), min(texCoord.y, 1.0 - texCoord.y));
            shade *= mix(0.77, 1.0, smoothstep(0.0, 0.075, edge));
            float dust = grain(localUV() * 46.0);
            shade *= 0.985 + dust * 0.03;
            shade += vec3(0.94, 0.94, 0.89) * pow(nDotH, vehicle ? 150.0 : 96.0)
                * visibility * (vehicle ? 0.58 : 0.38);
        }
        else if (uSurface == 16) // Automotive paint: broad clearcoat on curved panels.
        {
            float fresnel = 0.035 + 0.11 * pow(1.0 - nDotV, 5.0);
            shade = mix(shade, SkyColor(reflect(-viewDirection, normal)), fresnel);
            shade += vec3(0.95, 0.94, 0.88) * pow(nDotH, 58.0) * visibility * 0.34;
            shade += vec3(0.91, 0.92, 0.91) * pow(nDotH, 18.0) * visibility * 0.035;
        }
        else if (uSurface == 17) // Chrome retains a dark reflected underside.
        {
            vec3 reflected = SkyColor(reflect(-viewDirection, normal));
            float fresnel = 0.58 + 0.33 * pow(1.0 - nDotV, 5.0);
            shade = mix(shade * 0.80, reflected * color * 1.17, fresnel);
            shade += vec3(0.97, 0.98, 0.94) * pow(nDotH, 180.0) * visibility * 0.86;
        }
        else if (uSurface == 9)
            shade += vec3(0.93, 0.94, 0.90) * pow(nDotH, 28.0) * visibility * 0.035;
        else if (uSurface == 22)
        {
            // Small moving slopes reflect the cached sky; no riverbed/interior is required.
            vec2 river = worldPosition.xz;
            float waveX = 0.038 * cos(river.x * 0.85 + river.y * 0.21 + uTime * 0.70)
                + 0.017 * cos(river.x * 1.70 - river.y * 0.37 - uTime * 0.43);
            float waveZ = 0.025 * cos(river.y * 1.10 - river.x * 0.25 - uTime * 0.53)
                + 0.012 * cos(river.x * 0.63 + river.y * 1.60 + uTime * 0.39);
            vec3 waterNormal = normalize(vec3(-waveX, 1.0, -waveZ));
            float waterFacing = max(dot(waterNormal, viewDirection), 0.0);
            float fresnel = 0.065 + 0.65 * pow(1.0 - waterFacing, 5.0);
            float shore = smoothstep(0.0, 0.17, min(texCoord.x, 1.0 - texCoord.x));
            vec3 shallow = mix(vec3(0.22, 0.36, 0.31), color, shore);
            vec3 reflection = SkyColor(reflect(-viewDirection, waterNormal));
            shade = mix(shallow * vec3(0.82, 0.94, 0.98), reflection, fresnel);
            float ripple = sin(river.x * 3.4 + river.y * 1.7 - uTime * 0.85);
            shade *= 0.99 + ripple * 0.012;
            float sparkle = pow(max(dot(waterNormal, halfVector), 0.0), 115.0);
            shade += vec3(0.94, 0.93, 0.80) * sparkle * visibility * 0.42;
        }
    }

    float haze = smoothstep(42.0, 170.0, distance(uCameraPosition, worldPosition)) * 0.72;
    // Local architecture keeps its existing depth; only the scenery fully fades
    // into the cubemap horizon before the distant ground reaches its outer edge.
    vec3 hazeColor = mix(uHazeColor, vec3(0.75, 0.81, 0.84), 0.70);
    if (!uUseAssetMaterials && uSurface >= 21 && uSurface <= 24)
    {
        float sceneryDistance = distance(uCameraPosition, worldPosition);
        float aerial = smoothstep(70.0, 210.0, sceneryDistance) * 0.32;
        float farHaze = smoothstep(170.0, 450.0, sceneryDistance);
        haze = 1.0 - (1.0 - aerial) * (1.0 - farHaze);
        vec2 horizontal = worldPosition.xz - uCameraPosition.xz;
        horizontal = length(horizontal) > 0.001 ? normalize(horizontal) : vec2(0.0, 1.0);
        hazeColor = SkyColor(vec3(horizontal.x, 0.0, horizontal.y));
    }
    fragColor = vec4(mix(shade, hazeColor, haze), 1.0);
}
