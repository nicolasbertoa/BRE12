#include "../ShaderUtils/CBuffers.hlsli"
#include "../ShaderUtils/Lighting.hlsli"
#include "../ShaderUtils/Utils.hlsli"

//#define BRDF_FROSTBITE_ILLUMINANCE 
#define BRDF_FROSTBITE_LUMINANCE

struct Input {
	float4 mPosH : SV_POSITION;
	float3 mPosV : VIEW_RAY;
	nointerpolation PunctualLight mPunctualLight : PUNCTUAL_LIGHT;
};

ConstantBuffer<ImmutableCBuffer> gImmutableCBuffer : register(b0);

Texture2D<float2> NormalV : register (t0);
Texture2D<float4> BaseColor_MetalMask : register (t1);
Texture2D<float> Smoothness : register (t2);
Texture2D<float> Depth : register (t3);

struct Output {
	float4 mColor : SV_Target0;
};

Output main(const in Input input) {
	Output output = (Output)0;

	const int3 screenCoord = int3(input.mPosH.xy, 0);
	
	// Reconstruct geometry position in view space
	const float normalizedDepth = Depth.Load(screenCoord);
	const float farZ = gImmutableCBuffer.mNearZ_FarZ_ScreenW_ScreenH.y;
	const float3 viewRay = float3(input.mPosV.xy * (farZ / input.mPosV.z), farZ);
	const float3 geomPosV = viewRay * normalizedDepth;

	PunctualLight light = input.mPunctualLight;

	const float3 normalV = normalize(Decode(NormalV.Load(screenCoord)));

	const float4 baseColor_metalmask = BaseColor_MetalMask.Load(screenCoord);
	const float smoothness = Smoothness.Load(screenCoord);
	const float3 lightDirV = normalize(light.mLightPosVAndRange.xyz - geomPosV);
	// As we are working at view space, we do not need camera position to 
	// compute vector from geometry position to camera.
	const float3 viewV = normalize(-geomPosV);

	float3 luminance;
#ifdef BRDF_FROSTBITE_LUMINANCE
	luminance = computePunctualLightFrostbiteLuminance(light, geomPosV, normalV);
#else
	luminance = computePunctualLightDirectLuminance(light, geomPosV, normalV, 0.0000f);
#endif

	// Discard if luminance does not contribute any light.
	uint cond = dot(luminance, 1.0f) == 0;
	clip(cond * -1 + (1 - cond));

	float3 illuminance;
#ifdef BRDF_FROSTBITE_ILLUMINANCE
	illuminance = brdf_FrostBite(normalV, viewV, lightDirV, baseColor_metalmask.xyz, smoothness, baseColor_metalmask.w);
#else
	illuminance = brdf_CookTorrance(normalV, viewV, lightDirV, baseColor_metalmask.xyz, smoothness, baseColor_metalmask.w);
#endif

	// Discard if illuminance does not contribute any light.
	cond = dot(illuminance, 1.0f) == 0;
	clip(cond * -1 + (1 - cond));

	output.mColor = float4(luminance * illuminance, 1.0f);

	return output;
}