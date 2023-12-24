#pragma once
#include "../FormatsAndTypes.h"
#include "../PhysicalDevice.h"
#include "../Surface.h"
#include "volk.h"
#include <vector>
#include "../Object.h"
#include "../Buffer.h"
#include "../CommandAllocator.h"
#include "../CommandList.h"
#include "../CommandQueue.h"
#include "../Device.h"
#include "../DescriptorHeap.h"
#include "../Fence.h"
#include "../Instance.h"
namespace RHI
{
    enum QueueFamilyIndicesFlags
    {

        HasGraphics = 1 << 1,
        HasCompute = 1 << 2,
        HasPresent = 1 << 3,
        HasCopy = 1 << 4
    };
    DEFINE_ENUM_FLAG_OPERATORS(QueueFamilyIndicesFlags);
    struct QueueFamilyIndices
    {
        std::uint32_t graphicsIndex;
        std::uint32_t computeIndex;
        std::uint32_t copyIndex;
        std::uint32_t presentIndex;
        QueueFamilyIndicesFlags flags;
    };

    QueueFamilyIndices findQueueFamilyIndices(RHI::PhysicalDevice* device, RHI::Surface surface = RHI::Surface());


    VkFormat FormatConv(RHI::Format format);
    VkPrimitiveTopology vkPrimitiveTopology(PrimitiveTopology topology);
    extern PFN_vkSetPrivateDataEXT SetPrivateData;
    extern PFN_vkGetPrivateDataEXT GetPrivateData;

    class vResource
    {
    public:
        std::uint64_t offset;
        std::uint64_t size;
        Internal_ID heap;
    };
    class vBuffer : public Buffer, public vResource
    {
    };
    class vCommandAllocator : public CommandAllocator
    {

    };
    class vDevice : public Device
    {
    public:
        std::vector<HeapProperties> HeapProps;
        std::uint32_t DefaultHeapIndex = UINT32_MAX;
        std::uint32_t UploadHeapIndex = UINT32_MAX;
        std::uint32_t ReadbackHeapIndex = UINT32_MAX;
        VkPrivateDataSlot slot;
        QueueFamilyIndices indices;
    };
    class vCommandQueue : public CommandQueue
    {

    };
    class vGraphicsCommandList : public GraphicsCommandList
    {
    public:
        Internal_ID m_allocator;
    };
    class vDescriptorHeap : public DescriptorHeap
    {

    };
    class vFence : public Fence
    {

    };
    class vHeap : public Heap
    {

    };
    class vDescriptorSetLayout : public DescriptorSetLayout
    {

    };
    class vRootSignature : public RootSignature
    {

    };
    class vPipelineStateObject : public PipelineStateObject
    {

    };
    class vDescriptorSet : public DescriptorSet
    {

    };
    class vTexture : public Texture, public vResource
    {

    };
    class vInstance : public Instance
    {

    };
    class vPhysicalDevice : public PhysicalDevice
    {

    };
    class vSwapChain : public SwapChain
    {
    public:
        VkSemaphore present_semaphore;
        VkQueue PresentQueue_ID;
    };
}
