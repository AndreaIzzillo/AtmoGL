#ifndef ATMOSPHERE_COMMON_GLSL
#define ATMOSPHERE_COMMON_GLSL

#include "common.glsl"

const float PI = 3.14159265359;

layout(binding = 0) uniform sampler2D uTransmittanceLut;

uniform float uGroundRadius;
uniform float uTopRadius;

uniform float uRayleighScaleHeight;
uniform float uMieScaleHeight;
uniform float uOzoneCenterHeight;
uniform float uOzoneHalfWidth;

uniform vec3 uRayleighScattering;
uniform vec3 uMieScattering;

uniform vec3 uRayleighExtinction;
uniform vec3 uMieExtinction;
uniform vec3 uOzoneExtinction;

uniform vec3 uSunDirection;
uniform vec3 uSunIlluminance;
uniform float uMieAsymmetry;

struct MediumSample
{
    vec3 rayleighScattering;
    vec3 mieScattering;
    vec3 extinction;
};

struct AtmosphereResult
{
    vec3 radiance;
    vec3 transmittance;
};

MediumSample evaluateMedium(float altitude)
{
    MediumSample medium;
    medium.rayleighScattering = vec3(0.0);
    medium.mieScattering = vec3(0.0);
    medium.extinction = vec3(0.0);

    float atmosphereHeight = uTopRadius - uGroundRadius;

    if (altitude < 0.0 || altitude > atmosphereHeight)
        return medium;

    float rayleighDensity = exp(-altitude / uRayleighScaleHeight);
    float mieDensity = exp(-altitude / uMieScaleHeight);

    float ozoneDensity = max(
        0.0,
        1.0 - abs(altitude - uOzoneCenterHeight) / uOzoneHalfWidth
    );

    medium.rayleighScattering = rayleighDensity * uRayleighScattering;
    medium.mieScattering = mieDensity * uMieScattering;
    medium.extinction = rayleighDensity * uRayleighExtinction
                      + mieDensity * uMieExtinction
                      + ozoneDensity * uOzoneExtinction;

    return medium;
}

vec3 evaluateExtinction(float altitude)
{
    return evaluateMedium(altitude).extinction;
}

float rayleighPhase(float cosTheta)
{
    cosTheta = clamp(cosTheta, -1.0, 1.0);
    return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
}

float miePhase(float cosTheta, float g)
{
    cosTheta = clamp(cosTheta, -1.0, 1.0);

    float g2 = g * g;

    float factor = 3.0 / (8.0 * PI);
    float numLeft = 1.0 - g2;
    float numRight = 1.0 + cosTheta * cosTheta;
    float denomLeft = 2.0 + g2;
    float denomRight = 1.0 + g2 - 2.0 * g * cosTheta;

    return factor * (numLeft * numRight) / (denomLeft * (denomRight * sqrt(denomRight)));
}

float distanceToAtmosphereExit(float r, float mu)
{
    float rMu = r * mu;

    float discriminant =
        rMu * rMu
        + (uTopRadius - r) * (uTopRadius + r);

    return max(
        0.0,
        -rMu + sqrt(max(0.0, discriminant))
    );
}

float distanceToGround(float r, float mu)
{
    float rMu = r * mu;

    float discriminant =
        rMu * rMu
        - (r - uGroundRadius) * (r + uGroundRadius);

    return max(
        0.0,
        -rMu - sqrt(max(0.0, discriminant))
    );
}

bool rayIntersectsGround(float r, float mu)
{
    if (mu >= 0.0)
        return false;

    float rMu = r * mu;

    float discriminant =
        rMu * rMu
        - (r - uGroundRadius) * (r + uGroundRadius);

    return discriminant >= 0.0;
}

vec2 unitToTextureUv(vec2 coordinates, ivec2 lutSize)
{
    vec2 size = vec2(lutSize);
    return (vec2(0.5) + coordinates * (size - vec2(1.0))) / size;
}

vec2 textureUvToUnit(vec2 uv, ivec2 lutSize)
{
    vec2 size = vec2(lutSize);
    return (uv - vec2(0.5) / size) / (vec2(1.0) - vec2(1.0) / size);
}

vec2 transmittanceUvToRMu(vec2 uv, ivec2 lutSize)
{
    vec2 coordinates = clamp(textureUvToUnit(uv, lutSize), 0.0, 1.0);

    float H = sqrt(
        (uTopRadius - uGroundRadius) * (uTopRadius + uGroundRadius)
    );
    float rho = H * coordinates.y;
    float r = sqrt(rho * rho + uGroundRadius * uGroundRadius);
    r = clamp(r, uGroundRadius, uTopRadius);

    float dMin = uTopRadius - r;
    float dMax = rho + H;
    float d = mix(dMin, dMax, coordinates.x);

    float mu = 1.0;
    if (d > 0.0)
    {
        mu = ((uTopRadius - r) * (uTopRadius + r) - d * d)
             / (2.0 * r * d);
    }

    return vec2(r, clamp(mu, -1.0, 1.0));
}

vec2 transmittanceRMuToUv(float r, float mu, ivec2 lutSize)
{
    float H = sqrt(
        (uTopRadius - uGroundRadius) * (uTopRadius + uGroundRadius)
    );
    float rho = sqrt(max(
        0.0, (r - uGroundRadius) * (r + uGroundRadius)
    ));

    float dMin = uTopRadius - r;
    float dMax = rho + H;
    float d = distanceToAtmosphereExit(r, mu);

    vec2 coordinates = vec2(
        (d - dMin) / (dMax - dMin),
        rho / H
    );

    return unitToTextureUv(clamp(coordinates, 0.0, 1.0), lutSize);
}

vec3 skyViewUvToDirection(vec2 uv, vec3 cameraPosition, ivec2 lutSize)
{
    float r = length(cameraPosition);
    vec3 up = cameraPosition / r;

    float rho = sqrt(max(
        0.0,
        (r - uGroundRadius) * (r + uGroundRadius)
    ));
    float horizonAngle = acos(clamp(-rho / r, -1.0, 1.0));

    float y = clamp(textureUvToUnit(uv, lutSize).y, 0.0, 1.0);
    float zenithAngle;

    if (y < 0.5)
    {
        float coord = 1.0 - 2.0 * y;
        zenithAngle = horizonAngle * (1.0 - coord * coord);
    }
    else
    {
        float coord = 2.0 * y - 1.0;
        zenithAngle = horizonAngle + (PI - horizonAngle) * coord * coord;
    }

    float azimuth = 2.0 * PI * uv.x;

    vec3 reference = abs(up.x) < 0.99
        ? vec3(1.0, 0.0, 0.0)
        : vec3(0.0, 0.0, 1.0);
    vec3 tangentX = normalize(reference - dot(reference, up) * up);
    vec3 tangentZ = cross(tangentX, up);

    return normalize(
        cos(zenithAngle) * up
        + sin(zenithAngle) * (
            cos(azimuth) * tangentX
            + sin(azimuth) * tangentZ
        )
    );
}

vec2 skyViewDirectionToUv(vec3 direction, vec3 cameraPosition, ivec2 lutSize)
{
    direction = normalize(direction);

    float r = length(cameraPosition);
    vec3 up = cameraPosition / r;

    float rho = sqrt(max(
        0.0,
        (r - uGroundRadius) * (r + uGroundRadius)
    ));
    float horizonAngle = acos(clamp(-rho / r, -1.0, 1.0));

    vec3 reference = abs(up.x) < 0.99
        ? vec3(1.0, 0.0, 0.0)
        : vec3(0.0, 0.0, 1.0);
    vec3 tangentX = normalize(reference - dot(reference, up) * up);
    vec3 tangentZ = cross(tangentX, up);

    vec2 horizontal = vec2(dot(direction, tangentX), dot(direction, tangentZ));
    float zenithAngle = atan(length(horizontal), dot(up, direction));
    float y;

    if (zenithAngle < horizonAngle)
    {
        float coord = 1.0 - zenithAngle / horizonAngle;
        y = 0.5 * (1.0 - sqrt(clamp(coord, 0.0, 1.0)));
    }
    else
    {
        float coord = (zenithAngle - horizonAngle) / (PI - horizonAngle);
        y = 0.5 * (1.0 + sqrt(clamp(coord, 0.0, 1.0)));
    }

    float azimuth = 0.0;

    if (dot(horizontal, horizontal) > 1e-12)
        azimuth = atan(horizontal.y, horizontal.x);

    float u = fract(azimuth / (2.0 * PI));
    if (u >= 1.0)
        u = 0.0;
    float v = unitToTextureUv(vec2(0.0, y), lutSize).y;

    return vec2(u, v);
}

AtmosphereResult integrateAtmosphere(
    vec3 cameraPosition,
    vec3 viewDirection,
    float maxDistance,
    bool useTerrainShadows
)
{
    AtmosphereResult result;
    result.radiance = vec3(0.0);
    result.transmittance = vec3(1.0);

    float r = length(cameraPosition);
    vec3 up = cameraPosition / r;
    float mu = clamp(dot(up, viewDirection), -1.0, 1.0);
    float distanceToBoundary = distanceToAtmosphereExit(r, mu);

    if (rayIntersectsGround(r, mu))
    {
        distanceToBoundary = min(distanceToBoundary, distanceToGround(r, mu));
    }

    distanceToBoundary = min(distanceToBoundary, maxDistance);

    if (distanceToBoundary <= 0.0)
        return result;

    float cosTheta = dot(viewDirection, uSunDirection);
    float phaseRayleigh = rayleighPhase(cosTheta);
    float phaseMie = miePhase(cosTheta, uMieAsymmetry);

    vec3 radiance = vec3(0.0);
    vec3 opticalDepth = vec3(0.0);

    const int sampleCount = 30;
    float stepLength = distanceToBoundary / float(sampleCount);
    ivec2 lutSize = textureSize(uTransmittanceLut, 0);

    for (int i = 0; i < sampleCount; ++i)
    {
        float t = (float(i) + 0.5) * stepLength;
        vec3 position = cameraPosition + t * viewDirection;

        float sampleRadius = length(position);
        float altitude = sampleRadius - uGroundRadius;
        MediumSample medium = evaluateMedium(altitude);

        vec3 sampleUp = position / sampleRadius;
        float muSun = clamp(dot(sampleUp, uSunDirection), -1.0, 1.0);

        vec3 sunTransmittance = vec3(0.0);
        if (!rayIntersectsGround(sampleRadius, muSun))
        {
            // S(x, li) = Vis(li) * T(x, x + tatmoli)
            vec2 uv = transmittanceRMuToUv(sampleRadius, muSun, lutSize);
            float visibility = 1.0;
            if (useTerrainShadows)
            {
                vec3 worldPosition = position - vec3(0.0, uGroundRadius, 0.0);
                visibility = computeSunVisibility(worldPosition);
            }
            sunTransmittance = visibility * textureLod(uTransmittanceLut, uv, 0.0).rgb;
        }

        // σs(x) * p(v,li)
        vec3 scattering = medium.rayleighScattering * phaseRayleigh + medium.mieScattering * phaseMie;

        // Ei * σs(x) * p(v,li) * S(x, li)
        vec3 source = uSunIlluminance * scattering * sunTransmittance;

        // T(c, x)
        vec3 segmentOpticalDepth = medium.extinction * stepLength;
        vec3 transmissionAtMidpoint = exp(-(opticalDepth + 0.5 * segmentOpticalDepth));

        // Lscat(c, x, v) = T(c, x) * Ei * σs(x) * p(v,li) * S(x, li)
        vec3 Lscat = transmissionAtMidpoint * source;

        // L(c, v) = ∫(Lscat(c, x, v))
        radiance += Lscat * stepLength;

        opticalDepth += segmentOpticalDepth;
    }

    result.radiance = radiance;
    result.transmittance = exp(-opticalDepth);

    return result;
}

#endif
