#pragma once

#include <DirectXMath.h>

#include <CommandManager\CommandListPerFrame.h>
#include <MathUtils\MathUtils.h>
#include <ResourceManager\FrameCBufferPerFrame.h>
#include <ResourceManager/VertexAndIndexBufferCreator.h>

struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;
struct FrameCBuffer;
struct ID3D12CommandAllocator;
struct ID3D12Resource;
struct ID3D12GraphicsCommandList;

class SkyBoxCmdListRecorder {
public:
	SkyBoxCmdListRecorder() = default;
	~SkyBoxCmdListRecorder() = default;
	SkyBoxCmdListRecorder(const SkyBoxCmdListRecorder&) = delete;
	const SkyBoxCmdListRecorder& operator=(const SkyBoxCmdListRecorder&) = delete;
	SkyBoxCmdListRecorder(SkyBoxCmdListRecorder&&) = delete;
	SkyBoxCmdListRecorder& operator=(SkyBoxCmdListRecorder&&) = delete;

	// This method is to initialize PSO that is a shared between all this kind
	// of recorders.
	// This method is initialized by its corresponding pass.
	static void InitPSO() noexcept;

	void Init(
		const VertexAndIndexBufferCreator::VertexBufferData& vertexBufferData, 
		const VertexAndIndexBufferCreator::IndexBufferData indexBufferData,
		const DirectX::XMFLOAT4X4& worldMatrix,
		ID3D12Resource& skyBoxCubeMap,
		const D3D12_CPU_DESCRIPTOR_HANDLE& outputBufferCpuDesc,
		const D3D12_CPU_DESCRIPTOR_HANDLE& depthBufferCpuDesc) noexcept;

	// Preconditions:
	// - Init() must be called first
	void RecordAndPushCommandLists(const FrameCBuffer& frameCBuffer) noexcept;

	bool IsDataValid() const noexcept;

private:
	void InitConstantBuffers() noexcept;
	void InitShaderResourceViews(ID3D12Resource& skyBoxCubeMap) noexcept;

	CommandListPerFrame mCommandListPerFrame;
	
	VertexAndIndexBufferCreator::VertexBufferData mVertexBufferData;
	VertexAndIndexBufferCreator::IndexBufferData mIndexBufferData;
	DirectX::XMFLOAT4X4 mWorldMatrix{ MathUtils::GetIdentity4x4Matrix() };

	FrameCBufferPerFrame mFrameCBufferPerFrame;

	UploadBuffer* mObjectCBuffer{ nullptr };
	D3D12_GPU_DESCRIPTOR_HANDLE mObjectCBufferGpuDescBegin;

	D3D12_GPU_DESCRIPTOR_HANDLE mCubeMapBufferGpuDescBegin;

	D3D12_CPU_DESCRIPTOR_HANDLE mOutputColorBufferCpuDesc{ 0UL };
	D3D12_CPU_DESCRIPTOR_HANDLE mDepthBufferCpuDesc{ 0UL };
};