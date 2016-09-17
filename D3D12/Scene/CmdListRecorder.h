#pragma once

#include <cstdint>
#include <d3d12.h>
#include <DirectXMath.h>
#include <tbb/concurrent_queue.h>
#include <wrl.h>

#include <DXUtils\CBuffers.h>
#include <DXUtils/D3DFactory.h>
#include <GlobalData/Settings.h>
#include <MathUtils/MathUtils.h>
#include <ResourceManager/BufferCreator.h>
#include <Utils/DebugUtils.h>

class UploadBuffer;

// Responsible of command lists recording to be executed by CommandListProcessor.
// Steps:
// - Inherit from CmdListRecorder and reimplement RecordCommandLists() method
// - Call CmdListRecorder::RecordCommandLists() to create command lists to execute in the GPU
class CmdListRecorder {
public:
	struct GeometryData {
		GeometryData() = default;

		BufferCreator::VertexBufferData mVertexBufferData;
		BufferCreator::IndexBufferData mIndexBufferData;
		std::vector<DirectX::XMFLOAT4X4> mWorldMatrices;
	};

	explicit CmdListRecorder(ID3D12Device& device, tbb::concurrent_queue<ID3D12CommandList*>& cmdListQueue);
			
	// Build command lists and push them to the queue.
	virtual void RecordCommandLists(
		const FrameCBuffer& frameCBuffer,
		const D3D12_CPU_DESCRIPTOR_HANDLE* rtvCpuDescHandles,
		const std::uint32_t rtvCpuDescHandlesCount,
		const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilHandle) noexcept = 0;

	// This method validates all data (nullptr's, etc)
	// When you inherit from this class, you should reimplement it to include
	// new members
	virtual bool ValidateData() const noexcept;

protected:
	ID3D12Device& mDevice;
	tbb::concurrent_queue<ID3D12CommandList*>& mCmdListQueue;

	ID3D12GraphicsCommandList* mCmdList{ nullptr };
	ID3D12CommandAllocator* mCmdAlloc[Settings::sQueuedFrameCount]{ nullptr };
	std::uint32_t mCurrFrameIndex{ 0U };

	// Base command data. Once you inherits from this class, you should add
	// more class members that represent the extra information you need (like resources, for example)
	ID3D12DescriptorHeap* mCbvSrvUavDescHeap{ nullptr };
	ID3D12RootSignature* mRootSign{ nullptr };
	ID3D12PipelineState* mPSO{ nullptr };
	D3D12_PRIMITIVE_TOPOLOGY_TYPE mTopology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
	D3D12_VIEWPORT mScreenViewport{ 0.0f, 0.0f, (float)Settings::sWindowWidth, (float)Settings::sWindowHeight, 0.0f, 1.0f };
	D3D12_RECT mScissorRect{ 0, 0, Settings::sWindowWidth, Settings::sWindowHeight };
};