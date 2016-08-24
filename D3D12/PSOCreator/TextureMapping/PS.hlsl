#include "../ShaderUtils/Utils.hlsli"

#define FAR_PLANE_DISTANCE 5000.0f

struct Input {
	float4 mPosH : SV_POSITION;
	float3 mPosV : POS_VIEW;
	float3 mNormalV : NORMAL_VIEW;
	float2 mTexCoordO : TEXCOORD;
};

struct Material {
	float4 mBaseColor_MetalMask;
	float4 mReflectance_Smoothness;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct FrameConstants {
	float4x4 mV;
};
ConstantBuffer<FrameConstants> gFrameConstants : register(b1);

SamplerState TexSampler : register (s0);
Texture2D DiffuseTexture : register (t0);

struct Output {
	float2 mNormalV : SV_Target0;
	float4 mBaseColor_MetalMask : SV_Target1;
	float4 mReflectance_Smoothness : SV_Target2;
	float mDepthV : SV_Target3;
};

Output main(const in Input input) {
	Output output = (Output)0;
	const float3 normal = normalize(input.mNormalV);
	output.mNormalV = Encode(normal);
	const float3 diffuseColor = DiffuseTexture.Sample(TexSampler, input.mTexCoordO).rgb;
	output.mBaseColor_MetalMask = float4(gMaterial.mBaseColor_MetalMask.xyz * diffuseColor, gMaterial.mBaseColor_MetalMask.w);
	output.mReflectance_Smoothness = gMaterial.mReflectance_Smoothness;
	output.mDepthV = input.mPosV.z / FAR_PLANE_DISTANCE;

	return output;
}