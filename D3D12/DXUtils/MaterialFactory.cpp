#include "MaterialFactory.h"

#include <Utils\DebugUtils.h>

namespace {
	static Material sMaterials[MaterialFactory::NUM_MATERIALS]{};
}

void MaterialFactory::InitMaterials() noexcept {
	const float smoothness{ 0.95f };

	Material* m = &sMaterials[GOLD];
	m->mBaseColor_MetalMask[0U] = 1.0f;
	m->mBaseColor_MetalMask[1U] = 0.71f;
	m->mBaseColor_MetalMask[2U] = 0.29f;
	m->mBaseColor_MetalMask[3U] = 1.0f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[SILVER];
	m->mBaseColor_MetalMask[0U] = 0.95f;
	m->mBaseColor_MetalMask[1U] = 0.93f;
	m->mBaseColor_MetalMask[2U] = 0.88f;
	m->mBaseColor_MetalMask[3U] = 1.0f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[COPPER];
	m->mBaseColor_MetalMask[0U] = 0.95f;
	m->mBaseColor_MetalMask[1U] = 0.64f;
	m->mBaseColor_MetalMask[2U] = 0.54f;
	m->mBaseColor_MetalMask[3U] = 1.0f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[IRON];
	m->mBaseColor_MetalMask[0U] = 0.56f;
	m->mBaseColor_MetalMask[1U] = 0.57f;
	m->mBaseColor_MetalMask[2U] = 0.58f;
	m->mBaseColor_MetalMask[3U] = 1.0f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[ALUMINUM];
	m->mBaseColor_MetalMask[0U] = 0.91f;
	m->mBaseColor_MetalMask[1U] = 0.92f;
	m->mBaseColor_MetalMask[2U] = 0.92f;
	m->mBaseColor_MetalMask[3U] = 1.0f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[PLASTIC_GLASS_LOW];
	m->mBaseColor_MetalMask[3U] = 0.0f;
	m->mReflectance_Smoothness[0U] = 0.03f;
	m->mReflectance_Smoothness[1U] = 0.03f;
	m->mReflectance_Smoothness[2U] = 0.03f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[PLASTIC_HIGH];
	m->mBaseColor_MetalMask[3U] = 0.0f;
	m->mReflectance_Smoothness[0U] = 0.05f;
	m->mReflectance_Smoothness[1U] = 0.05f;
	m->mReflectance_Smoothness[2U] = 0.05f;
	m->mReflectance_Smoothness[3U] = smoothness;

	m = &sMaterials[GLASS_HIGH];
	m->mBaseColor_MetalMask[3U] = 0.0f;
	m->mReflectance_Smoothness[0U] = 0.08f;
	m->mReflectance_Smoothness[1U] = 0.08f;
	m->mReflectance_Smoothness[2U] = 0.08f;
	m->mReflectance_Smoothness[3U] = smoothness;
}

const Material& MaterialFactory::GetMaterial(const MaterialType material) noexcept {
	ASSERT(material < NUM_MATERIALS);
	return sMaterials[material];
}