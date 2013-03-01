#include "shared_structs.hlsl"
#include "constant_blocks.hlsl"
#include "vtstructs.hlsl"

//--------------------------------------------------------------------------------------
// Bounds computation utilities, similar to PointLightBounds.cpp
void UpdateClipRegionRoot(float nc,          // Tangent plane x/y normal coordinate (view space)
                          float lc,          // Light x/y coordinate (view space)
                          float lz,          // Light z coordinate (view space)
                          float lightRadius,
                          float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
                          inout float clipMin,
                          inout float clipMax)
{
    float nz = (lightRadius - nc * lc) / lz;
    float pz = (lc * lc + lz * lz - lightRadius * lightRadius) /
               (lz - (nz / nc) * lc);

    [flatten] if (pz > 0.0f) {
        float c = -nz * cameraScale / nc;
        [flatten] if (nc > 0.0f) {        // Left side boundary
            clipMin = max(clipMin, c);
        } else {                          // Right side boundary
            clipMax = min(clipMax, c);
        }
    }
}

void UpdateClipRegion(float lc,          // Light x/y coordinate (view space)
                      float lz,          // Light z coordinate (view space)
                      float lightRadius,
                      float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
                      inout float clipMin,
                      inout float clipMax)
{
    float rSq = lightRadius * lightRadius;
    float lcSqPluslzSq = lc * lc + lz * lz;
	float d = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

    if (d > 0) {
        float a = lightRadius * lc;
        float b = sqrt(d);
        float nx0 = (a + b) / lcSqPluslzSq;
        float nx1 = (a - b) / lcSqPluslzSq;
        
        UpdateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
        UpdateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
    }
}

// Returns bounding box [min.xy, max.xy] in clip [-1, 1] space.
float4 ComputeClipRegion(float3 lightPosView, float lightRadius)
{
    // Early out with empty rectangle if the light is too far behind the view frustum
    float4 clipRegion = float4(1, 1, 0, 0);
    if (lightPosView.z + lightRadius >= zPlanes.x) {
        float2 clipMin = float2(-1.0f, -1.0f);
        float2 clipMax = float2( 1.0f,  1.0f);
    
        UpdateClipRegion(lightPosView.x, lightPosView.z, lightRadius, matrixProjection._11, clipMin.x, clipMax.x);
        UpdateClipRegion(lightPosView.y, lightPosView.z, lightRadius, matrixProjection._22, clipMin.y, clipMax.y);

        clipRegion = float4(clipMin, clipMax);
    }

    return clipRegion;
}

// One per quad - gets expanded in the geometry shader
struct VS_OUT {
    float4 coords : coords;         // [min.xy, max.xy] in clip space
    float quadZ : quadZ;
    uint lightIndex : lightIndex;
};

VS_OUT main( uint lightIndex : SV_VertexID ) {
    VS_OUT output;
    output.lightIndex = lightIndex;

    // Work out tight clip-space rectangle
    VtSpotLight light = lightStore[lightIndex];
    float3 lightPositionView = mul( lightStore[lightIndex].position, matrixViewIT ).xyz;
    output.coords = ComputeClipRegion( lightPositionView, light.attenuationEnd );
    
    // Work out nearest depth for quad Z
    // Clamp to near plane in case this light intersects the near plane... don't want our quad to be clipped
    float quadDepth = max(zPlanes.x, lightPositionView.z - light.attenuationEnd );

    // Project quad depth into clip space
    float4 quadClip = mul( float4(0.0f, 0.0f, quadDepth, 1.0f), matrixProjection );
    output.quadZ = quadClip.z / quadClip.w;

    return output;
}