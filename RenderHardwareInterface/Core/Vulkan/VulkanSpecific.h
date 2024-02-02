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
#include "../ShaderReflect.h"
#include "../TextureView.h"
#include "vk_mem_alloc.h"
#include "spirv_reflect.h"
namespace RHI
{
    //todo find a better of doing this, because this would make the rhi single-device(gpu)
    //maybe make it an array or vector of allocators for multiple devices??
    
    extern VmaAllocator vma_allocator;
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

    class vDevice : public Device
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyDevice((VkDevice)ID, nullptr);
        }
    public:
        std::vector<HeapProperties> HeapProps;
        VmaAllocator allocator;
        std::uint32_t DefaultHeapIndex = UINT32_MAX;
        std::uint32_t UploadHeapIndex = UINT32_MAX;
        std::uint32_t ReadbackHeapIndex = UINT32_MAX;
        QueueFamilyIndices indices;
    };

    VkFormat FormatConv(RHI::Format format);
    VkPrimitiveTopology vkPrimitiveTopology(PrimitiveTopology topology);
    class vResource
    {
    public:
        std::uint64_t offset;
        std::uint64_t size;
        Internal_ID heap;
        VmaAllocation vma_ID = 0;//for automatic resources
    };
    class vBuffer : public Buffer, public vResource
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyBuffer( (VkDevice) ((vDevice*)device)->ID, (VkBuffer)Buffer::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vCommandAllocator : public CommandAllocator
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            if (((vCommandAllocator*)this)->m_pools.size())
            {
                vkFreeCommandBuffers((VkDevice)((vDevice*)device)->ID, (VkCommandPool)CommandAllocator::ID, m_pools.size(), (VkCommandBuffer*)m_pools.data());
            }
            vkDestroyCommandPool((VkDevice)((vDevice*)device)->ID, (VkCommandPool)CommandAllocator::ID, nullptr);
            ((vDevice*)device)->Release();
        }
        std::vector<Internal_ID> m_pools;
    };
    class vTextureView : public TextureView
    {

    };
    class vCommandQueue : public CommandQueue
    {
    };
    class vGraphicsCommandList : public GraphicsCommandList
    {
    };
    class vDynamicDescriptor : public DynamicDescriptor
    {

    };
    class vDescriptorHeap : public DescriptorHeap
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyDescriptorPool((VkDevice)((vDevice*)device)->ID, (VkDescriptorPool)DescriptorHeap::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vFence : public Fence
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroySemaphore((VkDevice)((vDevice*)device)->ID, (VkSemaphore)Fence::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vHeap : public Heap
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkFreeMemory((VkDevice)((vDevice*)device)->ID, (VkDeviceMemory)Heap::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vDescriptorSetLayout : public DescriptorSetLayout
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyDescriptorSetLayout((VkDevice)((vDevice*)device)->ID, (VkDescriptorSetLayout)DescriptorSetLayout::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vRootSignature : public RootSignature
    {
    public:
        //in d3d12 binding is done based on root param index, in vulkan its done based on set index
        //we store a vector of uint32_t which would correspond to the set index of each root param
        std::vector<uint32_t> setIndices;
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyPipelineLayout((VkDevice)((vDevice*)device)->ID, (VkPipelineLayout)RootSignature::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vPipelineStateObject : public PipelineStateObject
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyPipeline((VkDevice)((vDevice*)device)->ID, (VkPipeline)PipelineStateObject::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vDescriptorSet : public DescriptorSet
    {
    };
    class vTexture : public Texture, public vResource
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyImage((VkDevice)((vDevice*)device)->ID, (VkImage)Texture::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vInstance : public Instance
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroyInstance((VkInstance)Instance::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    };
    class vPhysicalDevice : public PhysicalDevice
    {
    };
    class vSwapChain : public SwapChain
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            vkDestroySwapchainKHR((VkDevice)((vDevice*)device)->ID, (VkSwapchainKHR)SwapChain::ID, nullptr);
            ((vDevice*)device)->Release();
        }
    public:
        VkSemaphore present_semaphore;
        VkQueue PresentQueue_ID;
    };
    class vShaderReflection : public ShaderReflection
    {
    public:
        virtual void Destroy() override
        {
            delete Object::refCnt;
            delete Object::ID;
        }
    };
}
