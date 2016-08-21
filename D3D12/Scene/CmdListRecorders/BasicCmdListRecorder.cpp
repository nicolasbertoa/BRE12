#include "BasicCmdListRecorder.h"

#include <DirectXMath.h>

#include <MathUtils/MathUtils.h>
#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/UploadBuffer.h>
#include <Utils/DebugUtils.h>

BasicCmdListRecorder::BasicCmdListRecorder(ID3D12Device& device, tbb::concurrent_queue<ID3D12CommandList*>& cmdListQueue)
	: CmdListRecorder(device, cmdListQueue)
{
}

void BasicCmdListRecorder::RecordCommandLists(
	const DirectX::XMFLOAT4X4& view,
	const DirectX::XMFLOAT4X4& proj,
	const D3D12_CPU_DESCRIPTOR_HANDLE* geomPassRtvCpuDescHandles,
	const std::uint32_t geomPassRtvCpuDescHandlesCount,
	const D3D12_CPU_DESCRIPTOR_HANDLE& depthStencilHandle) noexcept {
	ASSERT(ValidateData());
	ASSERT(geomPassRtvCpuDescHandles != nullptr);
	ASSERT(geomPassRtvCpuDescHandlesCount > 0);

	ID3D12CommandAllocator* cmdAlloc{ mCmdAlloc[mCurrFrameIndex] };
	ASSERT(cmdAlloc != nullptr);	
	
	// Update frame constants
	DirectX::XMFLOAT4X4 vp[2U];
	DirectX::XMStoreFloat4x4(&vp[0], MathUtils::GetTranspose(view));
	DirectX::XMStoreFloat4x4(&vp[1], MathUtils::GetTranspose(proj));
	UploadBuffer& frameCBuffer(*mFrameCBuffer[mCurrFrameIndex]);
	frameCBuffer.CopyData(0U, &vp, sizeof(vp));

	CHECK_HR(cmdAlloc->Reset());
	CHECK_HR(mCmdList->Reset(cmdAlloc, mPSO));

	mCmdList->RSSetViewports(1U, &mScreenViewport);
	mCmdList->RSSetScissorRects(1U, &mScissorRect);
	mCmdList->OMSetRenderTargets(geomPassRtvCpuDescHandlesCount, geomPassRtvCpuDescHandles, false, &depthStencilHandle);

	mCmdList->SetDescriptorHeaps(1U, &mCbvSrvUavDescHeap);
	mCmdList->SetGraphicsRootSignature(mRootSign);

	const std::size_t descHandleIncSize{ mDevice.GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) };
	D3D12_GPU_DESCRIPTOR_HANDLE objectCBufferGpuDescHandle(mObjectCBufferGpuDescHandleBegin);
	D3D12_GPU_DESCRIPTOR_HANDLE materialsCBufferGpuDescHandle(mMaterialsCBufferGpuDescHandleBegin);
		
	mCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set frame constants root parameters
	D3D12_GPU_VIRTUAL_ADDRESS frameCBufferGpuVAddress(frameCBuffer.Resource()->GetGPUVirtualAddress());
	mCmdList->SetGraphicsRootConstantBufferView(1U, frameCBufferGpuVAddress);
	mCmdList->SetGraphicsRootConstantBufferView(3U, frameCBufferGpuVAddress);

	// Draw objects
	const std::size_t geomCount{ mVertexAndIndexBufferDataVec.size() };
	for (std::size_t i = 0UL; i < geomCount; ++i) {
		mCmdList->IASetVertexBuffers(0U, 1U, &mVertexAndIndexBufferDataVec[i].first.mBufferView);
		mCmdList->IASetIndexBuffer(&mVertexAndIndexBufferDataVec[i].second.mBufferView);
		const std::size_t worldMatsCount{ mWorldMatrices[i].size() };
		for (std::size_t j = 0UL; j < worldMatsCount; ++j) {
			mCmdList->SetGraphicsRootDescriptorTable(0U, objectCBufferGpuDescHandle);
			objectCBufferGpuDescHandle.ptr += descHandleIncSize;

			mCmdList->SetGraphicsRootDescriptorTable(2U, materialsCBufferGpuDescHandle);
			materialsCBufferGpuDescHandle.ptr += descHandleIncSize;

			mCmdList->DrawIndexedInstanced(mVertexAndIndexBufferDataVec[i].second.mCount, 1U, 0U, 0U, 0U);
		}
	}
	
	mCmdList->Close();

	mCmdListQueue.push(mCmdList);

	// Next frame
	mCurrFrameIndex = (mCurrFrameIndex + 1) % Settings::sQueuedFrameCount;
}

bool BasicCmdListRecorder::ValidateData() const noexcept {
	for (std::size_t i = 0UL; i < mWorldMatrices.size(); ++i) {
		if (mWorldMatrices[i].empty()) {
			return false;
		}
	}

	for (std::uint32_t i = 0UL; i < Settings::sQueuedFrameCount; ++i) {
		if (mFrameCBuffer[i] == nullptr) {
			return false;
		}
	}

	const bool result = 
		CmdListRecorder::ValidateData() &&
		mObjectCBuffer != nullptr &&
		mObjectCBufferGpuDescHandleBegin.ptr != 0UL &&
		mVertexAndIndexBufferDataVec.empty() == false && 	
		mWorldMatrices.empty() == false &&
		mVertexAndIndexBufferDataVec.size() == mWorldMatrices.size() &&
		mMaterialsCBuffer != nullptr && 
		mMaterialsCBufferGpuDescHandleBegin.ptr != 0UL;

	return result;
}