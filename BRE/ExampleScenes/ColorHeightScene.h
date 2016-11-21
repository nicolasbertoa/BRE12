#pragma once

#include <Scene/Scene.h>

class ColorHeightScene : public Scene {
public:
	ColorHeightScene() = default;
	~ColorHeightScene() = default;
	ColorHeightScene(const ColorHeightScene&) = delete;
	const ColorHeightScene& operator=(const ColorHeightScene&) = delete;
	ColorHeightScene(ColorHeightScene&&) = delete;
	ColorHeightScene& operator=(ColorHeightScene&&) = delete;

	void Init(ID3D12CommandQueue& cmdQueue) noexcept final override;

	void GenerateGeomPassRecorders(
		std::vector<std::unique_ptr<GeometryPassCmdListRecorder>>& tasks) noexcept final override;

	void GenerateLightingPassRecorders(
		Microsoft::WRL::ComPtr<ID3D12Resource>* geometryBuffers,
		const std::uint32_t geometryBuffersCount,
		ID3D12Resource& depthBuffer,
		std::vector<std::unique_ptr<LightingPassCmdListRecorder>>& tasks) noexcept final override;

	void GenerateCubeMaps(
		ID3D12Resource* &skyBoxCubeMap,
		ID3D12Resource* &diffuseIrradianceCubeMap,
		ID3D12Resource* &specularPreConvolvedCubeMap) noexcept final override;
};