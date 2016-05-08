#pragma once

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl.h>

class ResourceManager {
public:
	static std::unique_ptr<ResourceManager> gResourceMgr;

	ResourceManager() = default;
	ResourceManager(const ResourceManager&) = delete;
	const ResourceManager& operator=(const ResourceManager&) = delete;

	// Returns resource id.
	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.
	std::size_t CreateDefaultBuffer(
		ID3D12Device& device,
		ID3D12GraphicsCommandList& cmdList,
		const void* initData,
		const std::uint64_t byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& defaultBuffer,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

	// Asserts if resource id is not present
	Microsoft::WRL::ComPtr<ID3D12Resource> GetResource(const std::size_t id);

	// This will invalidate all ids.
	void Clear() { mResources.resize(0); }

private:
	// Each index in the vector is the resource id
	using Resources = std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>;
	Resources mResources;
};
