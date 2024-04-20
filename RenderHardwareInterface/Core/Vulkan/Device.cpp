#include "pch.h"
#include "../Device.h"
#include "../../Error.h"
#include "VulkanSpecific.h"
#include "volk.h"
#define VMA_IMPLEMENTATION
#define VULKAN_AFTER_CRASH_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include "VulkanAfterCrash.h"
#include "vk_mem_alloc.h"
#include <iostream>
#include <fstream>
static void SelectHeapIndices(RHI::vDevice* device)
{
    std::uint32_t DefaultHeap = UINT32_MAX;
    std::uint32_t UploadHeap = UINT32_MAX;
    std::uint32_t ReadbackHeap = UINT32_MAX;
    int iterator = 0;
    for (auto& prop : device->HeapProps)
    {
        if (prop.memoryLevel ==  RHI::MemoryLevel::DedicatedRAM && iterator < DefaultHeap) DefaultHeap = iterator;
        if (prop.pageProperty == RHI::CPUPageProperty::WriteCached) UploadHeap = iterator;
        if (prop.pageProperty == RHI::CPUPageProperty::WriteCombined && iterator < UploadHeap) UploadHeap = iterator;
        if (prop.pageProperty == RHI::CPUPageProperty::WriteCombined || prop.pageProperty == RHI::CPUPageProperty::WriteCached && iterator < ReadbackHeap) ReadbackHeap = iterator;
        iterator++;
    }
    device->DefaultHeapIndex = DefaultHeap;
    device->UploadHeapIndex = UploadHeap;
    device->ReadbackHeapIndex = ReadbackHeap;
}

extern "C"
{
    RESULT RHI_API RHICreateDevice(RHI::PhysicalDevice* PhysicalDevice, RHI::CommandQueueDesc const* commandQueueInfos, int numCommandQueues, RHI::CommandQueue** commandQueues, Internal_ID instance, RHI::Device** device)
    {
        VkPhysicalDevice vkPhysicalDevice = (VkPhysicalDevice)PhysicalDevice->ID;

        VkPhysicalDeviceMemoryProperties memProps;
        RHI::vDevice* vdevice = new RHI::vDevice;
        RHI::vCommandQueue* vqueue = new RHI::vCommandQueue[numCommandQueues];
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProps);
        for (int i = 0; i < memProps.memoryTypeCount; i++)
        {
            VkFlags flags = memProps.memoryTypes[i].propertyFlags;
            RHI::HeapProperties prop;
            prop.type = RHI::HeapType::Custom;
            prop.memoryLevel = (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ? RHI::MemoryLevel::DedicatedRAM : RHI::MemoryLevel::SharedRAM;
            RHI::CPUPageProperty CPUprop;
            if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) CPUprop = RHI::CPUPageProperty::WriteCached;
            else if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) CPUprop = RHI::CPUPageProperty::WriteCombined;
            else CPUprop = RHI::CPUPageProperty::NonVisible;
            prop.pageProperty = CPUprop;
            vdevice->HeapProps.emplace_back(prop);
        }
        SelectHeapIndices(vdevice);
        vdevice->indices = findQueueFamilyIndices(PhysicalDevice);


        std::vector<VkDeviceQueueCreateInfo> queueInfos(numCommandQueues);
        for (int i = 0; i < numCommandQueues; i++)
        {
            queueInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfos[i].pNext = NULL;
            float queuePriority = commandQueueInfos[i].Priority;
            queueInfos[i].pQueuePriorities = &queuePriority;
            queueInfos[i].queueCount = 1;
            std::uint32_t index = 0;
            if (commandQueueInfos[i].CommandListType == RHI::CommandListType::Direct) index = vdevice->indices.graphicsIndex;
            if (commandQueueInfos[i].CommandListType == RHI::CommandListType::Compute) index = vdevice->indices.computeIndex;
            if (commandQueueInfos[i].CommandListType == RHI::CommandListType::Copy) index = vdevice->indices.copyIndex;
            queueInfos[i].queueFamilyIndex = index;
            queueInfos[i].flags = 0;
        }
        if (vdevice->indices.graphicsIndex != vdevice->indices.presentIndex)
        {
            VkDeviceQueueCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            float queuePriority = 0.f;
            info.pQueuePriorities = &queuePriority;
            info.queueCount = 1;
            info.queueFamilyIndex = vdevice->indices.presentIndex;
            info.flags = 0;
            queueInfos.push_back(info);
        }
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.shaderStorageImageReadWithoutFormat = VK_TRUE;
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

        VkPhysicalDeviceTimelineSemaphoreFeatures semaphoreFeatures{};
        semaphoreFeatures.timelineSemaphore = VK_TRUE;
        semaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
        semaphoreFeatures.pNext = &dynamicRenderingFeatures;

        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext = &semaphoreFeatures;
        info.pQueueCreateInfos = queueInfos.data();
        info.queueCreateInfoCount = queueInfos.size();
        info.pEnabledFeatures = &deviceFeatures;
        const char* ext_name[] =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
            VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
            VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME,
        };
        info.enabledExtensionCount = ARRAYSIZE(ext_name);
        info.ppEnabledExtensionNames = ext_name;
        VkResult res = vkCreateDevice(vkPhysicalDevice, &info, nullptr, (VkDevice*)&vdevice->ID);
        
       
        for (int i = 0; i < numCommandQueues; i++)
        {
            vkGetDeviceQueue((VkDevice)vdevice->ID, queueInfos[i].queueFamilyIndex, 0, (VkQueue*)&vqueue[i].ID);
            vqueue[i].device = vdevice;
            commandQueues[i] = &vqueue[i];
        }
        //initialize VMA
        VmaAllocatorCreateInfo vmaInfo{};
        vmaInfo.device = (VkDevice)vdevice->ID;
        vmaInfo.flags = 0; //probably have error checking
        vmaInfo.instance = (VkInstance)instance;
        vmaInfo.physicalDevice = (VkPhysicalDevice)PhysicalDevice->ID;
        vmaInfo.pVulkanFunctions = nullptr;
        
        vmaCreateAllocator(&vmaInfo, &vdevice->allocator);
        RHI::vma_allocator = vdevice->allocator;
        VkAfterCrash_DeviceCreateInfo acInfo;
        acInfo.flags = 0;
        acInfo.vkDevice = (VkDevice)vdevice->ID;
        acInfo.vkPhysicalDevice = (VkPhysicalDevice)PhysicalDevice->ID;
        VkAfterCrash_CreateDevice(&acInfo, &vdevice->acDevice);
        *device = vdevice;
        return res;
    }
}
namespace RHI
{   
    Default_t Default = {};
    Zero_t Zero = {};
    VmaAllocator RHI::vma_allocator = nullptr;
    static VkResult CreateShaderModule(const char* filename, VkPipelineShaderStageCreateInfo* shader_info, VkShaderStageFlagBits stage,int index, VkShaderModule* module,Internal_ID device)
    {
        char name[256];
        strcpy(name, filename);
        strcat(name, ".spv");
        std::vector<char> buffer;
        std::ifstream file(name, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        buffer.resize(size);
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), size);

        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = buffer.size();
        info.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
        vkCreateShaderModule((VkDevice)device, &info, nullptr, &module[index]);
        auto& vertShaderStageInfo = shader_info[index];
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = stage;
        vertShaderStageInfo.module = module[index];
        vertShaderStageInfo.pName = "main";
        
        return VK_SUCCESS;
    }
    static VkResult CreateShaderModule(const char* memory, uint32_t size, VkPipelineShaderStageCreateInfo* shader_info, VkShaderStageFlagBits stage, int index, VkShaderModule* module, Internal_ID device)
    {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = size;
        info.pCode = reinterpret_cast<const uint32_t*>(memory);
        vkCreateShaderModule((VkDevice)device, &info, nullptr, &module[index]);
        auto& vertShaderStageInfo = shader_info[index];
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = stage;
        vertShaderStageInfo.module = module[index];
        vertShaderStageInfo.pName = "main";
        return VK_SUCCESS;
    }
    RESULT Device::QueueWaitIdle(CommandQueue* queue)
    {
        return vkQueueWaitIdle((VkQueue)queue->ID);
    }
    static VkBufferUsageFlags VkBufferUsage(RHI::BufferUsage usage)
    {
        VkBufferUsageFlags flags = 0;
        if ((usage & RHI::BufferUsage::VertexBuffer) != BufferUsage::None)
        {
            flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if ((usage & RHI::BufferUsage::ConstantBuffer) != BufferUsage::None)
        {
            flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if ((usage & RHI::BufferUsage::IndexBuffer) != BufferUsage::None)
        {
            flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if ((usage & RHI::BufferUsage::CopySrc) != BufferUsage::None)
        {
            flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
        if ((usage & RHI::BufferUsage::CopyDst) != BufferUsage::None)
        {
            flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        if ((usage & RHI::BufferUsage::StructuredBuffer) != BufferUsage::None)
        {
            flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }
        return flags;
    }
    RESULT Device::CreateCommandAllocators(CommandListType type,uint32_t count,CommandAllocator** pAllocator)
    {
        vCommandAllocator* vallocator = new vCommandAllocator[3];
        std::uint32_t index = 0;
        if (type == CommandListType::Direct) index = ((vDevice*)this)->indices.graphicsIndex;
        if (type == CommandListType::Compute) index = ((vDevice*)this)->indices.computeIndex;
        if (type == CommandListType::Copy) index = ((vDevice*)this)->indices.copyIndex;
        VkCommandPoolCreateInfo info{};
        info.pNext = NULL;
        info.queueFamilyIndex = index;
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT; //to align with d3d's model
        VkResult res=VK_SUCCESS;
        for (uint32_t i = 0; i < count; i++)
        {
            res = vkCreateCommandPool((VkDevice)ID, &info, nullptr, (VkCommandPool*)&vallocator[i].ID);
            if (res != VK_SUCCESS) break;
            vallocator[i].device = this;
            Hold();
            pAllocator[i] = &vallocator[i];
        }
        return res;
    }
    template <> RESULT Device::CreateCommandList(CommandListType type, CommandAllocator* allocator, GraphicsCommandList** pCommandList)
    {
        vGraphicsCommandList* vCommandlist = new vGraphicsCommandList;
        VkCommandBufferAllocateInfo Info = {};
        Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        Info.commandPool = (VkCommandPool)allocator->ID;
        Info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        Info.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        VkResult res = vkAllocateCommandBuffers((VkDevice)ID, &Info, &commandBuffer);
        vCommandlist->ID = commandBuffer;
        vCommandlist->device = this;
        vCommandlist->allocator = ((vCommandAllocator*)allocator);
        *pCommandList = vCommandlist;
        Hold();
        ((vCommandAllocator*)allocator)->m_pools.push_back(vCommandlist->ID);
        
        return 0;
    }
    VkDescriptorType DescType(RHI::DescriptorType type)
    {
        switch (type)
        {
        case RHI::DescriptorType::SampledTexture: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            break;
        case RHI::DescriptorType::ConstantBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case RHI::DescriptorType::StructuredBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        case RHI::DescriptorType::ConstantBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            break;
        case RHI::DescriptorType::StructuredBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            break;
        case RHI::DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
            break;
        case RHI::DescriptorType::CSTexture: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            break;
        case RHI::DescriptorType::CSBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        default:
            break;
        }
    }
    RESULT Device::CreateDescriptorHeap(DescriptorHeapDesc* desc, DescriptorHeap** descriptorHeap)
    {
        vDescriptorHeap* vdescriptorHeap = new vDescriptorHeap;
        if (desc->poolSizes->type == DescriptorType::RTV || desc->poolSizes->type == DescriptorType::DSV)
        {
            vdescriptorHeap->ID = new VkImageView[desc->poolSizes->numDescriptors];
        }
        else if (desc->poolSizes->type == DescriptorType::Sampler)
        {
            vdescriptorHeap->ID = new VkSampler[desc->poolSizes->numDescriptors];
        }
        else
        {
            VkDescriptorPoolSize poolSize[5]{};
            for (uint32_t i = 0; i < desc->numPoolSizes; i++)
            {
                poolSize[i].type = DescType(desc->poolSizes[i].type);
                poolSize[i].descriptorCount = desc->poolSizes[i].numDescriptors;
            }

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = desc->numPoolSizes;
            poolInfo.pPoolSizes = poolSize;
            poolInfo.maxSets = desc->maxDescriptorSets;

            vkCreateDescriptorPool((VkDevice)ID, &poolInfo, nullptr, (VkDescriptorPool*)&vdescriptorHeap->ID);
        }
        *descriptorHeap = vdescriptorHeap;
        return 0;
    }
    RESULT Device::CreateRenderTargetView(Texture* texture, RenderTargetViewDesc* desc, CPU_HANDLE heapHandle)
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = NULL;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = (VkImage)texture->ID;
        info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = desc->textureMipSlice;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = desc->TextureArray ? desc->arraySlice: 0;
        info.subresourceRange.layerCount = 1;
        info.format = FormatConv(desc->format);
       return vkCreateImageView((VkDevice)ID, &info, nullptr, (VkImageView*)heapHandle.ptr);
        
    }
    RESULT Device::CreateDepthStencilView(Texture* texture, DepthStencilViewDesc* desc, CPU_HANDLE heapHandle)
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = NULL;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = (VkImage)texture->ID;
        info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        info.subresourceRange.baseMipLevel = desc->textureMipSlice;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = desc->TextureArray?desc->arraySlice:0;
        info.subresourceRange.layerCount = 1;
        info.format = FormatConv(desc->format);
        return vkCreateImageView((VkDevice)ID, &info, nullptr, (VkImageView*)heapHandle.ptr);
    }
    std::uint32_t Device::GetDescriptorHeapIncrementSize(DescriptorType type)
    {
        if(type == RHI::DescriptorType::RTV || type == RHI::DescriptorType::DSV)
            return sizeof(VkImageView);
        if (type == RHI::DescriptorType::Sampler)
            return sizeof(VkSampler);
    }
    RESULT Device::GetSwapChainImage(SwapChain* swapchain, std::uint32_t index, Texture** texture)
    {
        vTexture* vtexture = new vTexture;
        std::uint32_t img = index + 1;
        std::vector<VkImage> images(img);
        VkResult res = vkGetSwapchainImagesKHR((VkDevice)ID, (VkSwapchainKHR)swapchain->ID, &img, images.data());
        (vtexture)->ID = images[index];
        *texture = vtexture;
        return res;
    }
    RESULT Device::CreateFence(Fence** fence, std::uint64_t val)
    {
        vFence* vfence = new vFence;
        VkSemaphoreTypeCreateInfo timelineCreateInfo;
        timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineCreateInfo.pNext = NULL;
        timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineCreateInfo.initialValue = val;

        VkSemaphoreCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        createInfo.pNext = &timelineCreateInfo;
        createInfo.flags = 0;

        VkSemaphore timelineSemaphore;
        VkResult res = vkCreateSemaphore((VkDevice)ID, &createInfo, NULL, &timelineSemaphore);
        vfence->ID = timelineSemaphore;
        vfence->device = this;
        Hold();
        *fence = vfence;
        return res;
    }
    RESULT Device::CreateBuffer(BufferDesc* desc, Buffer** buffer, Heap* heap, HeapProperties* props, AutomaticAllocationInfo* automatic_info, std::uint64_t offset, ResourceType type)
    {
        vBuffer* vbuffer = new vBuffer;
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.size = desc->size;
        info.usage = VkBufferUsage(desc->usage);
        vbuffer->device = this;
        if (type == ResourceType::Automatic)
        {
            VmaAllocationCreateInfo allocCreateInfo{};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocCreateInfo.flags = 0;
            allocCreateInfo.flags |=
                (automatic_info->access_mode == AutomaticAllocationCPUAccessMode::Random) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;
            allocCreateInfo.flags |=
                (automatic_info->access_mode == AutomaticAllocationCPUAccessMode::Sequential) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0;
            RESULT res = vmaCreateBuffer(((vDevice*)this)->allocator, &info, &allocCreateInfo, (VkBuffer*)&vbuffer->ID, &vbuffer->vma_ID, nullptr);
            *buffer = vbuffer;
            Hold();
            return res;
        }
        vbuffer->offset = offset;
        vbuffer->size = desc->size;
        vbuffer->heap = heap->ID;
        vkCreateBuffer((VkDevice)ID, &info, nullptr, (VkBuffer*)&vbuffer->ID);
        if (type == ResourceType::Commited)
        {
            RHI::Heap* iheap;
            RHI::HeapDesc hdesc;
            RHI::MemoryReqirements req;
            GetBufferMemoryRequirements(desc, &req);
            hdesc.props = *props;
            hdesc.size = req.size;
            CreateHeap(&hdesc, &iheap, nullptr);
            vkBindBufferMemory((VkDevice)ID, (VkBuffer)vbuffer->ID, (VkDeviceMemory)iheap->ID, 0);
        }
        else if (type == ResourceType::Placed)
        {
            vkBindBufferMemory((VkDevice)ID, (VkBuffer)vbuffer->ID, (VkDeviceMemory)heap->ID, offset);
        }
        Hold();
        *buffer = vbuffer;
        return RESULT();
    }
    RESULT Device::GetBufferMemoryRequirements(BufferDesc* desc, MemoryReqirements* requirements)
    {
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.size = desc->size;
        info.usage = desc->usage == BufferUsage::VertexBuffer ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VkDeviceBufferMemoryRequirements DeviceReq{};
        DeviceReq.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;
        DeviceReq.pCreateInfo = &info;
        VkMemoryRequirements2 req{};
        req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        vkGetDeviceBufferMemoryRequirementsKHR((VkDevice)ID, &DeviceReq, &req);
        requirements->alignment = req.memoryRequirements.alignment;
        requirements->size = req.memoryRequirements.size;
        requirements->memoryTypeBits = req.memoryRequirements.memoryTypeBits;
        return RESULT();
    }
    RESULT Device::CreateHeap(HeapDesc* desc, Heap** heap, bool* usedFallback)
    {
        vHeap* vheap = new vHeap;
        if(usedFallback)*usedFallback = false;
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = desc->size;
        uint32_t MemIndex = UINT32_MAX;
        if (desc->props.type == HeapType::Custom)
        {
            uint32_t iterator = 0;
            for (auto& prop : ((vDevice*)this)->HeapProps)
            {
                if (prop.memoryLevel != desc->props.memoryLevel) { iterator++; continue; }
                if (prop.pageProperty != CPUPageProperty::NonVisible && desc->props.pageProperty == CPUPageProperty::Any)
                {
                    MemIndex = iterator;
                    iterator++;
                    continue;
                }
                if (prop.pageProperty != desc->props.pageProperty) {iterator++; continue;}
            }
            if (MemIndex == UINT32_MAX)
            {
                if(usedFallback)*usedFallback = true;
                if (desc->props.FallbackType == HeapType::Default)  MemIndex = ((vDevice*)this)->DefaultHeapIndex;
                if (desc->props.FallbackType == HeapType::Upload)   MemIndex = ((vDevice*)this)->UploadHeapIndex;
                if (desc->props.FallbackType == HeapType::Readback) MemIndex = ((vDevice*)this)->ReadbackHeapIndex;
            }
        }
        else
        {
            if (desc->props.type == HeapType::Default)  MemIndex = ((vDevice*)this)->DefaultHeapIndex;
            if (desc->props.type == HeapType::Upload)   MemIndex = ((vDevice*)this)->UploadHeapIndex;
            if (desc->props.type == HeapType::Readback) MemIndex = ((vDevice*)this)->ReadbackHeapIndex;
        }
        allocInfo.memoryTypeIndex = MemIndex;
        *heap = vheap;
        return vkAllocateMemory((VkDevice)ID, &allocInfo, nullptr, (VkDeviceMemory*)&vheap->ID);
    }
    VkShaderStageFlags VkShaderStage(ShaderStage stage)
    {
        VkShaderStageFlags flags = 0;
        if ((stage & ShaderStage::Vertex) != ShaderStage::None)
        {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if ((stage & ShaderStage::Pixel) != ShaderStage::None)
        {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if ((stage & ShaderStage::Geometry) != ShaderStage::None)
        {
            flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        if ((stage & ShaderStage::Hull) != ShaderStage::None)
        {
            flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if ((stage & ShaderStage::Domain) != ShaderStage::None)
        {
            flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if ((stage & ShaderStage::Compute) != ShaderStage::None)
        {
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        return flags;
    }
    RESULT Device::CreateRootSignature(RootSignatureDesc* desc, RootSignature** rootSignature, DescriptorSetLayout** pSetLayouts)
    {
        vDescriptorSetLayout* vSetLayouts = new vDescriptorSetLayout[desc->numRootParameters];
        vRootSignature* vrootSignature = new vRootSignature;
        VkDescriptorSetLayout descriptorSetLayout[20];
        //VkPushConstantRange pushConstantRanges[5];
        VkDescriptorSetLayoutCreateInfo layoutInfo[20]{};
        
        uint32_t minSetIndex = UINT32_MAX;
        uint32_t numLayouts = 0;
        for (uint32_t i = 0; i < desc->numRootParameters; i++)
        {
            if (desc->rootParameters[i].type == RootParameterType::DynamicDescriptor)
            {
                GFX_ASSERT(desc->rootParameters[i].dynamicDescriptor.type == RHI::DescriptorType::ConstantBufferDynamic
                || desc->rootParameters[i].dynamicDescriptor.type == RHI::DescriptorType::StructuredBufferDynamic);
                VkDescriptorSetLayoutBinding binding;
                binding.binding = 0;
                binding.descriptorCount = 1;
                binding.descriptorType = DescType(desc->rootParameters[i].dynamicDescriptor.type);
                binding.pImmutableSamplers = nullptr;
                binding.stageFlags = VkShaderStage(desc->rootParameters[i].dynamicDescriptor.stage);
                layoutInfo[i].bindingCount = 1;
                layoutInfo[i].pBindings = &binding;
                layoutInfo[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                vkCreateDescriptorSetLayout((VkDevice)ID, &layoutInfo[i], nullptr, &descriptorSetLayout[i]);
                continue;
            }
            VkDescriptorSetLayoutBinding LayoutBinding[20] = {};
            for (uint32_t j = 0; j < desc->rootParameters[i].descriptorTable.numDescriptorRanges; j++)
            {
                LayoutBinding[j].binding = desc->rootParameters[i].descriptorTable.ranges[j].BaseShaderRegister;
                LayoutBinding[j].descriptorCount = desc->rootParameters[i].descriptorTable.ranges[j].numDescriptors;
                LayoutBinding[j].descriptorType = DescType(desc->rootParameters[i].descriptorTable.ranges[j].type);
                LayoutBinding[j].pImmutableSamplers = nullptr;
                LayoutBinding[j].stageFlags = VkShaderStage(desc->rootParameters[i].descriptorTable.ranges[j].stage);
                numLayouts++;
            }
            layoutInfo[i].bindingCount = desc->rootParameters[i].descriptorTable.numDescriptorRanges;
            layoutInfo[i].pBindings = LayoutBinding;
            layoutInfo[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            vkCreateDescriptorSetLayout((VkDevice)ID, &layoutInfo[i], nullptr, &descriptorSetLayout[i]);
            pSetLayouts[i] = &vSetLayouts[i];
            vSetLayouts[i].device = this;
        }
        VkDescriptorSetLayout layoutInfoSorted[5]{};
        //sort descriptor sets
        for (uint32_t i = 0; i < desc->numRootParameters; i++)
        {
            if (desc->rootParameters[i].type == RootParameterType::DynamicDescriptor)
                layoutInfoSorted[desc->rootParameters[i].dynamicDescriptor.setIndex] = descriptorSetLayout[i];
            else if(desc->rootParameters[i].type == RootParameterType::DescriptorTable)
                layoutInfoSorted[desc->rootParameters[i].descriptorTable.setIndex] = descriptorSetLayout[i];
        }
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = desc->numRootParameters;
        pipelineLayoutInfo.pSetLayouts = layoutInfoSorted;
        VkResult res = vkCreatePipelineLayout((VkDevice)ID, &pipelineLayoutInfo, nullptr, (VkPipelineLayout*)&vrootSignature->ID);
        vrootSignature->device = this;
        *rootSignature = vrootSignature;
        for (uint32_t i = 0; i < desc->numRootParameters; i++)
        {
            if (desc->rootParameters[i].type == RootParameterType::DynamicDescriptor)
            {
                vkDestroyDescriptorSetLayout((VkDevice)ID, descriptorSetLayout[i], nullptr);
                continue;
            }
            vSetLayouts[i].ID = descriptorSetLayout[i];
        }
        return RESULT();
    }
    const VkCompareOp vkCompareFunc(RHI::ComparisonFunc func)
    {
        switch (func)
        {
        case RHI::ComparisonFunc::Never: return VK_COMPARE_OP_NEVER;
            break;
        case RHI::ComparisonFunc::Less: return VK_COMPARE_OP_LESS;
            break;
        case RHI::ComparisonFunc::Equal: return VK_COMPARE_OP_EQUAL;
            break;
        case RHI::ComparisonFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
            break;
        case RHI::ComparisonFunc::Greater: return VK_COMPARE_OP_GREATER;
            break;
        case RHI::ComparisonFunc::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            break;
        case RHI::ComparisonFunc::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            break;
        case RHI::ComparisonFunc::Always: return VK_COMPARE_OP_ALWAYS;
            break;
        default:
            break;
        }
    }
    const VkStencilOp vkStenOp(RHI::StencilOp op)
    {
        switch (op)
        {
        case RHI::StencilOp::Keep: return VK_STENCIL_OP_KEEP;
            break;
        case RHI::StencilOp::Zero: return VK_STENCIL_OP_ZERO;
            break;
        case RHI::StencilOp::Replace: return VK_STENCIL_OP_REPLACE;
            break;
        case RHI::StencilOp::IncrSat: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            break;
        case RHI::StencilOp::DecrSat: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            break;
        case RHI::StencilOp::Invert: return VK_STENCIL_OP_INVERT;
            break;
        case RHI::StencilOp::Incr: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
            break;
        case RHI::StencilOp::Decr: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
            break;
        default:
            break;
        }
    }
    VkCullModeFlags VkCullMode(CullMode mode)
    {
        switch (mode)
        {
        case RHI::CullMode::None: return VK_CULL_MODE_NONE;
            break;
        case RHI::CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
            break;
        case RHI::CullMode::Back: return VK_CULL_MODE_BACK_BIT;
            break;
        default: return VK_CULL_MODE_NONE;
            break;
        }
    }
    RESULT Device::CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject** pPSO)
    {
        vPipelineStateObject* vPSO = new vPipelineStateObject;
        GFX_ASSERT(desc->numInputElements < 5);
        VkPipelineShaderStageCreateInfo ShaderpipelineInfo[5] = {};
        VkShaderModule modules[5];
        int index = 0;
        if (desc->VS.data)
        {
            if(desc->shaderMode == File)
                CreateShaderModule(desc->VS.data, ShaderpipelineInfo, VK_SHADER_STAGE_VERTEX_BIT, index, modules, ID);
            else
                CreateShaderModule(desc->VS.data,desc->VS.size, ShaderpipelineInfo, VK_SHADER_STAGE_VERTEX_BIT, index, modules, ID);

            index++;
        }
        if (desc->PS.data) 
        {
            if (desc->shaderMode == File)
                CreateShaderModule(desc->PS.data, ShaderpipelineInfo, VK_SHADER_STAGE_FRAGMENT_BIT, index, modules, ID);
            else
                CreateShaderModule(desc->PS.data, desc->PS.size, ShaderpipelineInfo, VK_SHADER_STAGE_FRAGMENT_BIT, index, modules, ID);
            index++;
        }
        if (desc->GS.data)
        {
            if(desc->shaderMode == File)
                CreateShaderModule(desc->GS.data, ShaderpipelineInfo, VK_SHADER_STAGE_GEOMETRY_BIT, index, modules, ID);
            else
                CreateShaderModule(desc->GS.data, desc->GS.size, ShaderpipelineInfo, VK_SHADER_STAGE_GEOMETRY_BIT, index, modules, ID);
            index++;
        }
        if (desc->HS.data)
        {
            if (desc->shaderMode == File)
                CreateShaderModule(desc->HS.data, ShaderpipelineInfo, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, index,modules,ID);
            else
                CreateShaderModule(desc->HS.data,desc->HS.size, ShaderpipelineInfo, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, index,modules,ID);
            index++;
        }
        if (desc->DS.data)
        {
            if (desc->shaderMode == File)
                CreateShaderModule(desc->DS.data, ShaderpipelineInfo, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, index,modules, ID);
            else
                CreateShaderModule(desc->DS.data,desc->DS.size, ShaderpipelineInfo, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, index,modules, ID);
                
            index++;
        }
        VkVertexInputAttributeDescription inputAttribDesc[5];
        VkVertexInputBindingDescription inputbindingDesc[5];
        for (int i = 0; i < desc->numInputElements ; i++)
        {
            inputAttribDesc[i].format = FormatConv(desc->inputElements[i].format);
            inputAttribDesc[i].location = desc->inputElements[i].location;
            inputAttribDesc[i].offset = desc->inputElements[i].alignedByteOffset;
            inputAttribDesc[i].binding = desc->inputElements[i].inputSlot;

        }
        for (int i = 0; i < desc->numInputBindings; i++)
        {
            inputbindingDesc[i].binding = i;
            inputbindingDesc[i].inputRate = (VkVertexInputRate)desc->inputBindings[i].inputRate;
            inputbindingDesc[i].stride = desc->inputBindings[i].stride;
        }
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = desc->numInputBindings;
        vertexInputInfo.pVertexBindingDescriptions = inputbindingDesc; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = desc->numInputElements;
        vertexInputInfo.pVertexAttributeDescriptions = inputAttribDesc; // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = vkPrimitiveTopology(desc->rasterizerMode.topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,

        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = _ARRAYSIZE(dynamicStates);
        dynamicState.pDynamicStates = dynamicStates;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VkCullMode(desc->rasterizerMode.cullMode);
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.depthTestEnable = desc->depthStencilMode.DepthEnable;
        depthStencil.depthCompareOp = vkCompareFunc(desc->depthStencilMode.DepthFunc);
        depthStencil.depthWriteEnable = desc->depthStencilMode.DepthWriteMask == DepthWriteMask::All ? 1 : 0;
        //todo depth bounds
        depthStencil.flags = 0;
        depthStencil.stencilTestEnable = desc->depthStencilMode.StencilEnable;
        depthStencil.back.compareMask = desc->depthStencilMode.StencilReadMask;
        depthStencil.back.writeMask = desc->depthStencilMode.StencilWriteMask;
        depthStencil.back.compareOp = vkCompareFunc(desc->depthStencilMode.BackFace.Stencilfunc);
        depthStencil.back.depthFailOp = vkStenOp(desc->depthStencilMode.BackFace.DepthfailOp);
        depthStencil.back.failOp = vkStenOp(desc->depthStencilMode.BackFace.failOp);
        depthStencil.back.passOp = vkStenOp(desc->depthStencilMode.BackFace.passOp);
        depthStencil.front.compareMask = desc->depthStencilMode.StencilReadMask;
        depthStencil.front.writeMask = desc->depthStencilMode.StencilWriteMask;
        depthStencil.front.compareOp = vkCompareFunc(desc->depthStencilMode.FrontFace.Stencilfunc);
        depthStencil.front.depthFailOp = vkStenOp(desc->depthStencilMode.FrontFace.DepthfailOp);
        depthStencil.front.failOp = vkStenOp(desc->depthStencilMode.FrontFace.failOp);
        depthStencil.front.passOp = vkStenOp(desc->depthStencilMode.FrontFace.passOp);
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        VkPipelineRenderingCreateInfo pipelineRendereing{};
        pipelineRendereing.colorAttachmentCount = desc->numRenderTargets;//todo
        pipelineRendereing.depthAttachmentFormat = FormatConv(desc->DSVFormat);
        VkFormat format[10];
        for (uint32_t i = 0; i < desc->numRenderTargets; i++)
        {
            format[i] = FormatConv(desc->RTVFormats[0]);
        }
        pipelineRendereing.pColorAttachmentFormats = format;
        pipelineRendereing.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.pNext = &pipelineRendereing;
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = index;
        pipelineInfo.pStages = ShaderpipelineInfo;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = (VkPipelineLayout)desc->rootSig->ID;
        pipelineInfo.renderPass = nullptr;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional
        

        vPSO->device = this;
        Hold();
        vkCreateGraphicsPipelines((VkDevice)ID, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&vPSO->ID);
        *pPSO = vPSO;
        for(int i = 0; i < index; i++)
            vkDestroyShaderModule((VkDevice)ID, modules[i], nullptr);
        return RESULT();
    }
    RESULT Device::CreateDescriptorSets(DescriptorHeap* heap, std::uint32_t numDescriptorSets, DescriptorSetLayout* layouts, DescriptorSet** pSets)
    {
        vDescriptorSet* vSets = new vDescriptorSet[numDescriptorSets];
        VkDescriptorSetLayout vklayouts[5];
        for (uint32_t i = 0; i < numDescriptorSets; i++)
        {
            vklayouts[i] = (VkDescriptorSetLayout)layouts[i].ID;
        }
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = (VkDescriptorPool)heap->ID;
        allocInfo.descriptorSetCount = numDescriptorSets;
        allocInfo.pSetLayouts = vklayouts;
        VkDescriptorSet descriptorSets[5];
        VkResult res = vkAllocateDescriptorSets((VkDevice)ID, &allocInfo, descriptorSets);
        for (uint32_t i = 0; i < numDescriptorSets; i++)
        {
            vSets[i].ID = descriptorSets[i];
            pSets[i] = &vSets[i];
        }
        return res;
    }
    RESULT Device::UpdateDescriptorSets(std::uint32_t numDescs, DescriptorSetUpdateDesc* desc, DescriptorSet* sets)
    {
        
        VkWriteDescriptorSet writes[5]{};
        VkDescriptorBufferInfo Binfo[5]{ };
        VkDescriptorImageInfo Iinfo[5]{ };
        for(uint32_t i = 0; i < numDescs; i++)
        {
            switch (desc[i].type)
            {
            case(RHI::DescriptorType::CSTexture):
            {
                Iinfo[i].imageView = (VkImageView)desc[i].textureInfos->texture->ID;
                Iinfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                Iinfo[i].sampler = nullptr;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                writes[i].pImageInfo = &Iinfo[i];
                break;
            }
            case(RHI::DescriptorType::CSBuffer):
            {
                Binfo[i].buffer = (VkBuffer)desc[i].bufferInfos->buffer->ID;
                Binfo[i].offset = desc[i].bufferInfos->offset;
                Binfo[i].range = desc[i].bufferInfos->range;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writes[i].pBufferInfo = &Binfo[i];
                break;
            }
            case(RHI::DescriptorType::ConstantBuffer):
            {
                Binfo[i].buffer = (VkBuffer)desc[i].bufferInfos->buffer->ID;
                Binfo[i].offset = desc[i].bufferInfos->offset;
                Binfo[i].range = desc[i].bufferInfos->range;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[i].pBufferInfo = &Binfo[i];
                break;
            }
            case(RHI::DescriptorType::SampledTexture):
            {
                Iinfo[i].imageView = (VkImageView)desc[i].textureInfos->texture->ID;
                Iinfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                Iinfo[i].sampler = nullptr;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writes[i].pImageInfo = &Iinfo[i];
                break;
            }
            case(RHI::DescriptorType::StructuredBuffer):
            {
                Binfo[i].buffer = (VkBuffer)desc[i].bufferInfos->buffer->ID;
                Binfo[i].offset = desc[i].bufferInfos->offset;
                Binfo[i].range = desc[i].bufferInfos->range;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                writes[i].pBufferInfo = &Binfo[i];
                break;
            }
            case(RHI::DescriptorType::Sampler):
            {
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                Iinfo[i].sampler = *(VkSampler*)desc[i].samplerInfos->heapHandle.ptr;
                Iinfo[i].imageView = VK_NULL_HANDLE;
                writes[i].pImageInfo = &Iinfo[i];
            }
            default:
                break;
            }
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = (VkDescriptorSet)sets->ID;
            writes[i].dstBinding = desc[i].binding;
            writes[i].dstArrayElement = 0;
            writes[i].descriptorCount = desc->numDescriptors;
        }
        vkUpdateDescriptorSets((VkDevice)ID, numDescs, writes, 0, nullptr);
        
        return RESULT();
    }
    VkImageType ImageType(TextureType type)
    {
        if (type == TextureType::Texture1D) return VK_IMAGE_TYPE_1D;
        if (type == TextureType::Texture2D) return VK_IMAGE_TYPE_2D;
        if (type == TextureType::Texture3D) return VK_IMAGE_TYPE_3D;
    }
    VkImageTiling ImageTiling(TextureTilingMode mode)
    {
        if (mode == TextureTilingMode::Linear) return VK_IMAGE_TILING_LINEAR;
        if (mode == TextureTilingMode::Optimal) return VK_IMAGE_TILING_OPTIMAL;
    }
    VkImageUsageFlags ImageUsage(RHI::TextureUsage usage)
    {
        VkImageUsageFlags flags = 0;
        if ((usage & RHI::TextureUsage::SampledImage) != RHI::TextureUsage::None)
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if ((usage & RHI::TextureUsage::CopyDst) != RHI::TextureUsage::None)
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if ((usage & RHI::TextureUsage::ColorAttachment) != RHI::TextureUsage::None)
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if ((usage & RHI::TextureUsage::DepthStencilAttachment) != RHI::TextureUsage::None)
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if ((usage & RHI::TextureUsage::CopySrc) != RHI::TextureUsage::None)
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if ((usage & RHI::TextureUsage::StorageImage) != RHI::TextureUsage::None)
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;

        return flags;
    }
    RESULT Device::CreateDynamicDescriptor(DescriptorHeap* heap,DynamicDescriptor** descriptor, DescriptorType type,ShaderStage stage, RHI::Buffer* buffer,uint32_t offset,uint32_t size)
    {
        GFX_ASSERT(type == DescriptorType::ConstantBufferDynamic || type == DescriptorType::StructuredBufferDynamic);
        VkDescriptorSetLayoutBinding binding;
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = DescType(type);
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VkShaderStage(stage);
        VkDescriptorSetLayoutCreateInfo info;//todo construct the set layout only once as part of the device
        info.pNext = 0;
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.flags = 0;
        info.pBindings = &binding;
        VkDescriptorSetLayout layout;
        vDynamicDescriptor* vdescriptor = new vDynamicDescriptor();
        vkCreateDescriptorSetLayout((VkDevice)ID, &info, nullptr, &layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = (VkDescriptorPool)heap->ID;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;
        vkAllocateDescriptorSets((VkDevice)ID, &allocInfo, (VkDescriptorSet*)&vdescriptor->ID);
        *descriptor = vdescriptor;
        //vkDestroyDescriptorSetLayout((VkDevice)ID, layout, nullptr);
        VkDescriptorBufferInfo binfo{};
        binfo.buffer = (VkBuffer)buffer->ID;
        binfo.offset = offset;
        binfo.range = size;
        VkWriteDescriptorSet write{};
        write.descriptorCount = 1;
        write.descriptorType = DescType(type);
        write.dstSet = (VkDescriptorSet)vdescriptor->ID;
        write.pBufferInfo = &binfo;
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkUpdateDescriptorSets((VkDevice)ID, 1,&write, 0, nullptr);
        return RESULT();
    }
    RESULT Device::CreateTexture(TextureDesc* desc, Texture** texture, Heap* heap, HeapProperties* props, AutomaticAllocationInfo* automatic_info,std::uint64_t offset, ResourceType type)
    {
        vTexture* vtexture = new vTexture;
        VkImageCreateInfo info{};
        info.arrayLayers = desc->type == TextureType::Texture3D ? 1 : desc->depthOrArraySize;
        info.extent.width = desc->width;
        info.extent.height = desc->height;
        info.extent.depth = desc->type == TextureType::Texture3D ? desc->depthOrArraySize : 1;
        VkImageCreateFlags flags = 0;
        if ((desc->usage & TextureUsage::CubeMap) != TextureUsage::None) flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        info.flags = flags;
        info.format = FormatConv(desc->format);
        info.imageType = ImageType(desc->type);
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.mipLevels = desc->mipLevels;
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.tiling = ImageTiling(desc->mode);
        info.usage = ImageUsage(desc->usage);
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        vkCreateImage((VkDevice)ID, &info, nullptr, (VkImage*)&vtexture->ID);
        vtexture->device = this;
        if (type == ResourceType::Automatic)
        {
            VmaAllocationCreateInfo allocCreateInfo{};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocCreateInfo.flags = 0;
            allocCreateInfo.flags |=
                (automatic_info->access_mode == AutomaticAllocationCPUAccessMode::Random) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;
            allocCreateInfo.flags |=
                (automatic_info->access_mode == AutomaticAllocationCPUAccessMode::Sequential) ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0;
            RESULT res = vmaCreateImage(((vDevice*)this)->allocator, &info, &allocCreateInfo, (VkImage*)&vtexture->ID, &vtexture->vma_ID, nullptr);
            *texture = vtexture;
            return res;
        }
        if (type == ResourceType::Commited)
        {
            RHI::Heap* iheap;
            RHI::HeapDesc hdesc;
            RHI::MemoryReqirements req;
            GetTextureMemoryRequirements(desc, &req);
            hdesc.props = *props;
            hdesc.size = req.size;
            CreateHeap(&hdesc, &iheap, nullptr);
            vkBindImageMemory((VkDevice)ID, (VkImage)vtexture->ID, (VkDeviceMemory)iheap->ID, 0);
        }
        else if (type == ResourceType::Placed)
        {
            vkBindImageMemory((VkDevice)ID, (VkImage)vtexture->ID, (VkDeviceMemory)heap->ID, offset);
        }
        *texture = vtexture;
        return RESULT();
    }
    RESULT Device::GetTextureMemoryRequirements(TextureDesc* desc, MemoryReqirements* requirements)
    {
        VkImageCreateInfo info{};
        info.arrayLayers = desc->type == TextureType::Texture3D ? 1 : desc->depthOrArraySize;
        info.extent.width = desc->width;
        info.extent.height = desc->height;
        info.extent.depth = desc->type == TextureType::Texture3D ? desc->depthOrArraySize : 1;
        info.flags = 0;
        info.format = FormatConv(desc->format);
        info.imageType = ImageType(desc->type);
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.mipLevels = desc->mipLevels;
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.tiling = ImageTiling(desc->mode);
        info.usage = ImageUsage(desc->usage);
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        VkDeviceImageMemoryRequirements DeviceReq{};
        DeviceReq.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
        DeviceReq.pCreateInfo = &info;
        VkMemoryRequirements2 req{};
        req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        vkGetDeviceImageMemoryRequirementsKHR((VkDevice)ID, &DeviceReq, &req);
        requirements->alignment = req.memoryRequirements.alignment;
        requirements->size = req.memoryRequirements.size;
        requirements->memoryTypeBits = req.memoryRequirements.memoryTypeBits;
        return RESULT();
    }
    static VkImageViewType VkViewType(TextureViewType type)
    {
        switch (type)
        {
        case RHI::TextureViewType::Texture1D: return VK_IMAGE_VIEW_TYPE_1D;
            break;
        case RHI::TextureViewType::Texture2D: return VK_IMAGE_VIEW_TYPE_2D;
            break;
        case RHI::TextureViewType::Texture3D: return VK_IMAGE_VIEW_TYPE_3D;
            break;
        case RHI::TextureViewType::TextureCube: return VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        default:
            break;
        }
    }
    RESULT Device::CreateTextureView(TextureViewDesc* desc, TextureView** view)
    {
        VkImageViewCreateInfo info{};
        info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        info.flags = 0;
        info.format = FormatConv(desc->format);
        info.image = (VkImage)desc->texture->ID;
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        VkImageSubresourceRange range;
        range.aspectMask = (VkImageAspectFlags)desc->range.imageAspect;
        range.baseMipLevel =                   desc->range.IndexOrFirstMipLevel;
        range.baseArrayLayer =                 desc->range.FirstArraySlice;
        range.layerCount =                     desc->range.NumArraySlices;
        range.levelCount =                     desc->range.NumMipLevels;
        info.subresourceRange = range;
        info.viewType = VkViewType(desc->type);
        vTextureView* vtview = new vTextureView;
        vkCreateImageView((VkDevice)ID, &info, nullptr, (VkImageView*)&vtview->ID);
        *view = vtview;
        return RESULT();
    }
    VkSamplerAddressMode VkAddressMode(AddressMode mode)
    {
        switch (mode)
        {
        case RHI::AddressMode::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        case RHI::AddressMode::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case RHI::AddressMode::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        case RHI::AddressMode::Wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        default:
            break;
        }
    }
    VkFilter vkFilter(Filter mode)
    {
        return (mode == Filter::Linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    }
    VkSamplerMipmapMode vkMipMode(Filter mode)
    {
        return (mode == Filter::Linear) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
    RESULT Device::CreateSampler(SamplerDesc* desc, CPU_HANDLE heapHandle)
    {
        VkSamplerCreateInfo info;
        info.addressModeU = VkAddressMode(desc->AddressU);
        info.addressModeV = VkAddressMode(desc->AddressV);
        info.addressModeW = VkAddressMode(desc->AddressW);
        info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        info.anisotropyEnable = desc->anisotropyEnable;
        info.compareEnable = desc->compareEnable;
        info.compareOp = vkCompareFunc(desc->compareFunc);
        info.flags = 0;
        info.magFilter = vkFilter(desc->magFilter);
        info.maxAnisotropy = desc->maxAnisotropy;
        info.maxLod = desc->maxLOD == FLT_MAX ? VK_LOD_CLAMP_NONE : desc->maxLOD;
        info.minFilter = vkFilter(desc->minFilter);
        info.minLod = desc->minLOD;
        info.mipLodBias = desc->mipLODBias;
        info.mipmapMode = vkMipMode(desc->mipFilter);
        info.pNext = 0;
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.unnormalizedCoordinates = VK_FALSE;
        vkCreateSampler((VkDevice)ID, &info, nullptr, (VkSampler*)heapHandle.ptr);
        return RESULT();
    }
    RESULT Device::CreateComputePipeline(ComputePipelineDesc* desc, ComputePipeline** pCP)
    {
        vComputePipeline* vpipeline = new vComputePipeline;
        vpipeline->device = this;
        VkComputePipelineCreateInfo info{};
        info.layout = (VkPipelineLayout)desc->rootSig->ID;
        VkShaderModule module;
        if (desc->mode == ShaderMode::File) CreateShaderModule(desc->CS.data, &info.stage, VK_SHADER_STAGE_COMPUTE_BIT, 0, &module, ID);
        else CreateShaderModule(desc->CS.data, desc->CS.size, &info.stage, VK_SHADER_STAGE_COMPUTE_BIT, 0, &module, ID);
        info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        vkCreateComputePipelines((VkDevice)ID, VK_NULL_HANDLE, 1, &info, nullptr, (VkPipeline*)&vpipeline->ID);
        *pCP = vpipeline;
        vkDestroyShaderModule((VkDevice)ID, module, nullptr);
        Hold();
        return RESULT();
    }
    RESULT Device::CreateDebugBuffer(DebugBuffer** buffer)
    {
        vDebugBuffer* buff = new vDebugBuffer;
        VkAfterCrash_BufferCreateInfo info;
        info.markerCount = 1;
        VkAfterCrash_CreateBuffer(((vDevice*)this)->acDevice, &info, (VkAfterCrash_Buffer*)&buff->ID,&buff->data);
        *buffer = buff;
        return 0;
    }

}
