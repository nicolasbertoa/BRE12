#include "AmbientOcclusionPass.h"

#include <d3d12.h>

#include <CommandListExecutor/CommandListExecutor.h>
#include <DescriptorManager\CbvSrvUavDescriptorManager.h>
#include <DescriptorManager\RenderTargetDescriptorManager.h>
#include <DXUtils\D3DFactory.h>
#include <ResourceManager\ResourceManager.h>
#include <ResourceStateManager\ResourceStateManager.h>
#include <Utils\DebugUtils.h>

namespace BRE {
namespace {
///
/// @brief Creates resource, render target view and shader resource view.
/// @param resourceInitialState Initial statea of the resource to create
/// @param resourceName Name of the resource
/// @param resource Output resource
/// @param resourceRenderTargetView Output render target view to the resource
///
void
CreateResourceAndRenderTargetView(const D3D12_RESOURCE_STATES resourceInitialState,
                                  const wchar_t* resourceName,
                                  ID3D12Resource* &resource,
                                  D3D12_CPU_DESCRIPTOR_HANDLE& resourceRenderTargetView,
                                  D3D12_GPU_DESCRIPTOR_HANDLE& resourceShaderResourceView) noexcept
{
    BRE_ASSERT(resource == nullptr);

    // Set shared buffers properties
    const D3D12_RESOURCE_DESC resourceDescriptor = D3DFactory::GetResourceDescriptor(ApplicationSettings::sWindowWidth,
                                                                                     ApplicationSettings::sWindowHeight,
                                                                                     DXGI_FORMAT_R16_UNORM,
                                                                                     D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

    D3D12_CLEAR_VALUE clearValue{ resourceDescriptor.Format, 0.0f, 0.0f, 0.0f, 0.0f };

    const D3D12_HEAP_PROPERTIES heapProperties = D3DFactory::GetHeapProperties();

    // Create buffer resource
    resource = &ResourceManager::CreateCommittedResource(heapProperties,
                                                         D3D12_HEAP_FLAG_NONE,
                                                         resourceDescriptor,
                                                         resourceInitialState,
                                                         &clearValue,
                                                         resourceName,
                                                         ResourceManager::ResourceStateTrackingType::FULL_TRACKING);

    // Create render target view	
    D3D12_RENDER_TARGET_VIEW_DESC rtvDescriptor{};
    rtvDescriptor.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDescriptor.Format = resourceDescriptor.Format;
    RenderTargetDescriptorManager::CreateRenderTargetView(*resource,
                                                          rtvDescriptor,
                                                          &resourceRenderTargetView);

    // Create shader resource view
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescriptor{};
    srvDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescriptor.Texture2D.MostDetailedMip = 0;
    srvDescriptor.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDescriptor.Format = resource->GetDesc().Format;
    srvDescriptor.Texture2D.MipLevels = resource->GetDesc().MipLevels;
    resourceShaderResourceView = CbvSrvUavDescriptorManager::CreateShaderResourceView(*resource,
                                                                                      srvDescriptor);
}
}

void
AmbientOcclusionPass::Init(ID3D12Resource& normalRoughnessBuffer,
                           ID3D12Resource& depthBuffer,
                           const D3D12_GPU_DESCRIPTOR_HANDLE& normalRoughnessBufferShaderResourceView,
                           const D3D12_GPU_DESCRIPTOR_HANDLE& depthBufferShaderResourceView) noexcept
{
    BRE_ASSERT(IsDataValid() == false);

    AmbientOcclusionCommandListRecorder::InitSharedPSOAndRootSignature();
    BlurCommandListRecorder::InitSharedPSOAndRootSignature();

    // Create ambient accessibility buffer and blur buffer
    CreateResourceAndRenderTargetView(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                      L"Ambient Accessibility Buffer",
                                      mAmbientAccessibilityBuffer,
                                      mAmbientAccessibilityBufferRenderTargetView,
                                      mAmbientAccessibilityBufferShaderResourceView);

    CreateResourceAndRenderTargetView(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                      L"Blur Buffer",
                                      mBlurBuffer,
                                      mBlurBufferRenderTargetView,
                                      mBlurBufferShaderResourceView);

    // Initialize ambient occlusion recorder
    mAmbientOcclusionRecorder.Init(mAmbientAccessibilityBufferRenderTargetView,
                                   normalRoughnessBufferShaderResourceView,
                                   depthBufferShaderResourceView);

    // Initialize blur recorder
    mBlurRecorder.Init(mAmbientAccessibilityBufferShaderResourceView,
                       mBlurBufferRenderTargetView);

    mNormalRoughnessBuffer = &normalRoughnessBuffer;
    mDepthBuffer = &depthBuffer;

    BRE_ASSERT(IsDataValid());
}

std::uint32_t
AmbientOcclusionPass::Execute(const FrameCBuffer& frameCBuffer) noexcept
{
    BRE_ASSERT(IsDataValid());

    std::uint32_t commandListCount = 0U;

    commandListCount += RecordAndPushPrePassCommandLists();
    commandListCount += mAmbientOcclusionRecorder.RecordAndPushCommandLists(frameCBuffer);

    commandListCount += RecordAndPushMiddlePassCommandLists();
    commandListCount += mBlurRecorder.RecordAndPushCommandLists();

    return commandListCount;
}

bool
AmbientOcclusionPass::IsDataValid() const noexcept
{
    const bool b =
        mAmbientAccessibilityBuffer != nullptr &&
        mAmbientAccessibilityBufferShaderResourceView.ptr != 0UL &&
        mAmbientAccessibilityBufferRenderTargetView.ptr != 0UL &&
        mBlurBuffer != nullptr &&
        mBlurBufferShaderResourceView.ptr != 0UL &&
        mBlurBufferRenderTargetView.ptr != 0UL &&
        mNormalRoughnessBuffer != nullptr &&
        mDepthBuffer != nullptr;

    return b;
}

std::uint32_t
AmbientOcclusionPass::RecordAndPushPrePassCommandLists() noexcept
{
    BRE_ASSERT(IsDataValid());

    ID3D12GraphicsCommandList& commandList = mPrePassCommandListPerFrame.ResetCommandListWithNextCommandAllocator(nullptr);

    D3D12_RESOURCE_BARRIER barriers[4U];
    std::uint32_t barrierCount = 0UL;
    if (ResourceStateManager::GetResourceState(*mAmbientAccessibilityBuffer) != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        barriers[barrierCount] = ResourceStateManager::ChangeResourceStateAndGetBarrier(*mAmbientAccessibilityBuffer,
                                                                                        D3D12_RESOURCE_STATE_RENDER_TARGET);
        ++barrierCount;
    }

    if (ResourceStateManager::GetResourceState(*mBlurBuffer) != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        barriers[barrierCount] = ResourceStateManager::ChangeResourceStateAndGetBarrier(*mBlurBuffer,
                                                                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        ++barrierCount;
    }

    if (ResourceStateManager::GetResourceState(*mNormalRoughnessBuffer) != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        barriers[barrierCount] = ResourceStateManager::ChangeResourceStateAndGetBarrier(*mNormalRoughnessBuffer,
                                                                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        ++barrierCount;
    }

    if (ResourceStateManager::GetResourceState(*mDepthBuffer) != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        barriers[barrierCount] = ResourceStateManager::ChangeResourceStateAndGetBarrier(*mDepthBuffer,
                                                                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        ++barrierCount;
    }

    if (barrierCount > 0UL) {
        commandList.ResourceBarrier(barrierCount, barriers);
    }

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandList.ClearRenderTargetView(mAmbientAccessibilityBufferRenderTargetView,
                                      clearColor,
                                      0U,
                                      nullptr);

    BRE_CHECK_HR(commandList.Close());
    CommandListExecutor::Get().PushCommandList(commandList);

    return 1U;
}

std::uint32_t
AmbientOcclusionPass::RecordAndPushMiddlePassCommandLists() noexcept
{
    BRE_ASSERT(IsDataValid());


    ID3D12GraphicsCommandList& commandList = mMiddlePassCommandListPerFrame.ResetCommandListWithNextCommandAllocator(nullptr);

    D3D12_RESOURCE_BARRIER barriers[2U];
    std::uint32_t barrierCount = 0UL;
    if (ResourceStateManager::GetResourceState(*mAmbientAccessibilityBuffer) != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        barriers[barrierCount] = ResourceStateManager::ChangeResourceStateAndGetBarrier(*mAmbientAccessibilityBuffer,
                                                                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        ++barrierCount;
    }

    if (ResourceStateManager::GetResourceState(*mBlurBuffer) != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        barriers[barrierCount] = ResourceStateManager::ChangeResourceStateAndGetBarrier(*mBlurBuffer,
                                                                                        D3D12_RESOURCE_STATE_RENDER_TARGET);
        ++barrierCount;
    }

    if (barrierCount > 0UL) {
        commandList.ResourceBarrier(barrierCount, barriers);
    }

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    commandList.ClearRenderTargetView(mBlurBufferRenderTargetView,
                                      clearColor,
                                      0U,
                                      nullptr);

    BRE_CHECK_HR(commandList.Close());
    CommandListExecutor::Get().PushCommandList(commandList);

    return 1U;
}
}