#pragma once
#include "Core.h"
#include "../FormatsAndTypes.h"
#include "include\d3d12.h"
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include "../DescriptorHeap.h"
#include "../Instance.h"
#include "../Device.h"
#include "../CommandQueue.h"
#include "../PipelineStateObject.h"
#include "../RootSignature.h"
#include "../Fence.h"
#include <vector>
namespace RHI
{
    D3D12_BARRIER_ACCESS D3DAcessMode(ResourceAcessFlags flags);
    D3D12_BARRIER_LAYOUT D3DBarrierLayout(ResourceLayout layout);
    D3D12_BARRIER_SYNC   D3DBarrierSync(PipelineStage stage);

    class D3D12Device : public Device
    {
    };
    class D3D12CommandQueue : public CommandQueue
    {

    };
    class D3D12Instance : public Instance
    {

    };
    class D3D12Texture : public Texture
    {

    };
    class D3D12Heap : public Heap
    {

    };
    class D3D12RootSignature : public RootSignature
    {
    };
    class D3D12DescriptorSetLayout : public DescriptorSetLayout
    {
        friend class Device;
        std::uint32_t numBindings;
        std::uint32_t numDescriptors;
    };
    class D3D12Fence : public Fence
    {

    };
    class D3D12PipelineStateObject : public PipelineStateObject
    {
        friend class Device;
        friend class GraphicsCommandList;
        std::vector<std::uint32_t> strides;
    };
    class D3D12DescriptorHeap : public DescriptorHeap
    {
        friend class Device;
        UINT64 offset = { 0 };
    };
    class D3D12CommandAllocator : public CommandAllocator
    {
        
    };
    class D3D12GraphicsCommandList : public GraphicsCommandList
    {

    };
    class D3D12DescriptorSet : public DescriptorSet
    {
        friend class Device;
        friend class GraphicsCommandList;
        CPU_HANDLE start;
        std::uint64_t gpu_handle;
    };
    class D3D12Buffer : public Buffer
    {
        friend class Device;
    };
    class D3D12SwapChain : public SwapChain
    {

    };
}
