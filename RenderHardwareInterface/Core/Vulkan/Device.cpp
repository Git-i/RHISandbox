#include "pch.h"
#include "../Device.h"
#include "../../Error.h"
#include "VulkanSpecific.h"
#include "volk.h"
#include <iostream>
#include <fstream>
#include <csetjmp>
namespace RHI
{   
    Default_t Default = {};
    Zero_t Zero = {};
    PFN_vkSetPrivateDataEXT SetPrivateData = nullptr;
    PFN_vkGetPrivateDataEXT GetPrivateData = nullptr;
    static void SelectHeapIndices(vDevice* device)
    {
        std::uint32_t DefaultHeap = UINT32_MAX;
        std::uint32_t UploadHeap = UINT32_MAX;
        std::uint32_t ReadbackHeap = UINT32_MAX;
        int iterator = 0;
        for (auto& prop : device->HeapProps)
        {
            if (prop.memoryLevel == MemoryLevel::DedicatedRAM && iterator < DefaultHeap) DefaultHeap = iterator;
            if (prop.pageProperty == CPUPageProperty::WriteCached) UploadHeap = iterator;
            if (prop.pageProperty == CPUPageProperty::WriteCombined && iterator < UploadHeap) UploadHeap = iterator;
            if (prop.pageProperty == CPUPageProperty::WriteCombined || prop.pageProperty == CPUPageProperty::WriteCached && iterator < ReadbackHeap) ReadbackHeap = iterator;
            iterator++;
        }
        device->DefaultHeapIndex = DefaultHeap;
        device->UploadHeapIndex = UploadHeap;
        device->ReadbackHeapIndex = ReadbackHeap;
    }
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
        return flags;
    }
    RESULT Device::Create(PhysicalDevice* PhysicalDevice, CommandQueueDesc const* commandQueueInfos, int numCommandQueues, CommandQueue** commandQueues, Device** device)
    {
        VkPhysicalDevice vkPhysicalDevice = (VkPhysicalDevice)PhysicalDevice->ID;
        
        VkPhysicalDeviceMemoryProperties memProps;
        vDevice* vdevice = new vDevice;
        vCommandQueue* vqueue = new vCommandQueue[numCommandQueues];
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProps);
        for (int i = 0; i < memProps.memoryTypeCount; i++)
        {
            VkFlags flags = memProps.memoryTypes[i].propertyFlags;
            HeapProperties prop;
            prop.type = HeapType::Custom;
            prop.memoryLevel = (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) ? MemoryLevel::DedicatedRAM : MemoryLevel::SharedRAM;
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
            if (commandQueueInfos[i].CommandListType == CommandListType::Direct) index = vdevice->indices.graphicsIndex;
            if (commandQueueInfos[i].CommandListType == CommandListType::Compute) index = vdevice->indices.computeIndex;
            if (commandQueueInfos[i].CommandListType == CommandListType::Copy) index = vdevice->indices.copyIndex;
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

        VkPhysicalDevicePrivateDataFeatures features{};
        features.privateData = VK_TRUE;
        features.pNext = &semaphoreFeatures;
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES;

        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pNext = &features;
        info.pQueueCreateInfos = queueInfos.data();
        info.queueCreateInfoCount = queueInfos.size();
        info.pEnabledFeatures = &deviceFeatures;
        const char* ext_name[] = 
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
            VK_EXT_PRIVATE_DATA_EXTENSION_NAME, 
            VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, 
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, 
            VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
            VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
            VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME
        };
        info.enabledExtensionCount = ARRAYSIZE(ext_name);
        info.ppEnabledExtensionNames = ext_name;
        VkResult res = vkCreateDevice(vkPhysicalDevice, &info, nullptr, (VkDevice*)&vdevice->ID);
        
        typedef  VkResult(VKAPI_PTR * PFN_vkCreatePrivateDataSlotEXT)(VkDevice, const VkPrivateDataSlotCreateInfo*, const VkAllocationCallbacks*, VkPrivateDataSlot*);
        VkPrivateDataSlotCreateInfo Slotinfo = {};
        Slotinfo.sType = VK_STRUCTURE_TYPE_PRIVATE_DATA_SLOT_CREATE_INFO;
        PFN_vkCreatePrivateDataSlotEXT fn = (PFN_vkCreatePrivateDataSlotEXT)vkGetDeviceProcAddr((VkDevice)vdevice->ID, "vkCreatePrivateDataSlotEXT");
        SetPrivateData = (PFN_vkSetPrivateDataEXT)vkGetDeviceProcAddr((VkDevice)vdevice->ID, "vkSetPrivateDataEXT");
        GetPrivateData = (PFN_vkGetPrivateDataEXT)vkGetDeviceProcAddr((VkDevice)vdevice->ID, "vkGetPrivateDataEXT");
        vkCreatePrivateDataSlotEXT((VkDevice)vdevice->ID, &Slotinfo, nullptr, &vdevice->slot);
        for (int i = 0; i < numCommandQueues; i++)
        {
            vkGetDeviceQueue((VkDevice)vdevice->ID, queueInfos[i].queueFamilyIndex, 0, (VkQueue*)&vqueue[i].ID);
            vqueue[i].Device_ID = vdevice->ID;
            vqueue[i].PrivateDataSlot = vdevice->slot;
            commandQueues[i] = &vqueue[i];
        }
        *device = vdevice;
        return res;
    }
    RESULT Device::CreateCommandAllocator(CommandListType type,CommandAllocator** pAllocator)
    {
        vCommandAllocator* vallocator = new vCommandAllocator;
        std::uint32_t index = 0;
        if (type == CommandListType::Direct) index = ((vDevice*)this)->indices.graphicsIndex;
        if (type == CommandListType::Compute) index = ((vDevice*)this)->indices.computeIndex;
        if (type == CommandListType::Copy) index = ((vDevice*)this)->indices.copyIndex;
        VkCommandPoolCreateInfo info{};
        info.pNext = NULL;
        info.queueFamilyIndex = index;
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        vkCreateCommandPool((VkDevice)ID, &info, nullptr, (VkCommandPool*)&vallocator->ID);
        SetPrivateData((VkDevice)ID, VkObjectType::VK_OBJECT_TYPE_UNKNOWN, (std::uint64_t)vallocator->ID, ((vDevice*)this)->slot, 1);
        vallocator->PrivateDataSlot = ((vDevice*)this)->slot;
        vallocator->Device_ID = ID;
        *pAllocator = vallocator;
        return 0;
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
        SetPrivateData((VkDevice)ID, VkObjectType::VK_OBJECT_TYPE_COMMAND_BUFFER, (std::uint64_t)vCommandlist->ID, ((vDevice*)this)->slot, 1);
        vCommandlist->PrivateDataSlot = ((vDevice*)this)->slot;
        vCommandlist->Device_ID = ID;
        vCommandlist->m_allocator = allocator->ID;
        *pCommandList = vCommandlist;
        return 0;
    }
    VkDescriptorType DescType(RHI::DescriptorType type)
    {
        switch (type)
        {
        case RHI::DescriptorType::CBV: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case RHI::DescriptorType::RTV: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            break;
        case RHI::DescriptorType::DSV: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            break;
        case RHI::DescriptorType::SRV: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            break;
        case RHI::DescriptorType::UAV: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
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
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.format = VK_FORMAT_B8G8R8A8_UNORM; //todo
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
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.format = VK_FORMAT_D32_SFLOAT;
        return vkCreateImageView((VkDevice)ID, &info, nullptr, (VkImageView*)heapHandle.ptr);
    }
    std::uint32_t Device::GetDescriptorHeapIncrementSize(DescriptorType type)
    {
        return sizeof(VkImageView);
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
        SetPrivateData((VkDevice)ID, VkObjectType::VK_OBJECT_TYPE_SEMAPHORE, (std::uint64_t)vfence->ID, ((vDevice*)this)->slot, 1);
        vfence->PrivateDataSlot = ((vDevice*)this)->slot;
        vfence->Device_ID = ID;
        *fence = vfence;
        return res;
    }
    RESULT Device::CreateBuffer(BufferDesc* desc, Buffer** buffer, Heap* heap, HeapProperties* props, std::uint64_t offset, ResourceType type)
    {
        vBuffer* vbuffer = new vBuffer;
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.size = desc->size;
        info.usage = VkBufferUsage(desc->usage);
        vbuffer->offset = offset;
        vbuffer->size = desc->size;
        vbuffer->Device_ID = ID;
        vbuffer->PrivateDataSlot = ((vDevice*)this)->slot;
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
    
    RESULT Device::CreateRootSignature(RootSignatureDesc* desc, RootSignature** rootSignature, DescriptorSetLayout** pSetLayouts)
    {
        vDescriptorSetLayout* vSetLayouts = new vDescriptorSetLayout[desc->numRootParameters];
        vRootSignature* vrootSignature = new vRootSignature;
        VkDescriptorSetLayout descriptorSetLayout[5];
        //VkPushConstantRange pushConstantRanges[5];
        VkDescriptorSetLayoutCreateInfo layoutInfo[5]{};
        uint32_t numLayouts = 0;
        for (uint32_t i = 0; i < desc->numRootParameters; i++)
        {
            VkDescriptorSetLayoutBinding LayoutBinding[5] = {};
            for (uint32_t j = 0; j < desc->rootParameters[i].descriptorTable.numDescriptorRanges; j++)
            {
                LayoutBinding[j].binding = desc->rootParameters[i].descriptorTable.ranges[j].BaseShaderRegister;
                LayoutBinding[j].descriptorCount = desc->rootParameters[i].descriptorTable.ranges[j].numDescriptors;
                LayoutBinding[j].descriptorType = DescType(desc->rootParameters[i].descriptorTable.ranges[j].type);
                LayoutBinding[j].pImmutableSamplers = nullptr;
                LayoutBinding[j].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
                numLayouts++;
            }
            layoutInfo[i].bindingCount = desc->rootParameters[i].descriptorTable.numDescriptorRanges;
            layoutInfo[i].pBindings = LayoutBinding;
            layoutInfo[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            vkCreateDescriptorSetLayout((VkDevice)ID, &layoutInfo[i], nullptr, &descriptorSetLayout[i]);
            pSetLayouts[i] = &vSetLayouts[i];
            vSetLayouts[i].Device_ID = ID;
            vSetLayouts[i].PrivateDataSlot = ((vDevice*)this)->slot;
        }
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = desc->numRootParameters;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayout;
        vkCreatePipelineLayout((VkDevice)ID, &pipelineLayoutInfo, nullptr, (VkPipelineLayout*)&vrootSignature->ID);
        vrootSignature->Device_ID = ID;
        vrootSignature->PrivateDataSlot = ((vDevice*)this)->slot;
        *rootSignature = vrootSignature;
        for (uint32_t i = 0; i < desc->numRootParameters; i++)
            vSetLayouts[i].ID = descriptorSetLayout[i];
        return RESULT();
    }
    RESULT Device::CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject** pPSO)
    {
        vPipelineStateObject* vPSO = new vPipelineStateObject;
        GFX_ASSERT(desc->numInputElements < 5);
        VkPipelineShaderStageCreateInfo ShaderpipelineInfo[5] = {};
        VkShaderModule modules[5];
        int index = 0;
        if (desc->VS)
        {
            CreateShaderModule(desc->VS, ShaderpipelineInfo, VK_SHADER_STAGE_VERTEX_BIT, index, modules, ID);
            index++;
        }
        if (desc->PS) 
        {
            CreateShaderModule(desc->PS, ShaderpipelineInfo, VK_SHADER_STAGE_FRAGMENT_BIT, index, modules,ID);
            index++;
        }
        if (desc->GS)
        {
            CreateShaderModule(desc->GS, ShaderpipelineInfo, VK_SHADER_STAGE_GEOMETRY_BIT, index, modules, ID);
            index++;
        }
        if (desc->HS)
        {
            CreateShaderModule(desc->HS, ShaderpipelineInfo, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, index,modules,ID);
            index++;
        }
        if (desc->DS)
        {
            CreateShaderModule(desc->DS, ShaderpipelineInfo, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, index,modules, ID);
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
        inputAssembly.topology = vkPrimitiveTopology(desc->topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkDynamicState dynamicStates[2] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
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
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
        depthStencil.depthTestEnable = desc->DepthEnabled;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //todo
        depthStencil.depthWriteEnable = true;
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        VkPipelineRenderingCreateInfo pipelineRendereing{};
        pipelineRendereing.colorAttachmentCount = 1;//todo
        pipelineRendereing.depthAttachmentFormat = FormatConv(desc->DSVFormat);
        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        pipelineRendereing.pColorAttachmentFormats = &format;
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
        vkAllocateDescriptorSets((VkDevice)ID, &allocInfo, descriptorSets);
        for (uint32_t i = 0; i < numDescriptorSets; i++)
        {
            vSets[i].ID = descriptorSets[i];
            pSets[i] = &vSets[i];
        }
        return RESULT();
    }
    RESULT Device::UpdateDescriptorSets(std::uint32_t numDescs, DescriptorSetUpdateDesc* desc, DescriptorSet* sets)
    {
        VkWriteDescriptorSet writes[5]{};
        VkDescriptorBufferInfo Binfo[5]{ };
        VkDescriptorImageInfo Iinfo[5]{ };
        VkImageView view[5]{};
        for(uint32_t i = 0; i < numDescs; i++)
        {
            if (desc[i].type == RHI::DescriptorType::CBV)
            {
                Binfo[i].buffer = (VkBuffer)desc[i].bufferInfos->buffer->ID;
                Binfo[i].offset = desc[i].bufferInfos->offset;
                Binfo[i].range = desc[i].bufferInfos->range;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                writes[i].pBufferInfo = &Binfo[i];
            }
            if (desc[i].type == RHI::DescriptorType::SRV)
            {
                VkImageViewCreateInfo info{};
                info.viewType = VK_IMAGE_VIEW_TYPE_2D; //todo
                info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                info.image = (VkImage)desc[i].textureInfos->texture->ID;
                info.format = VK_FORMAT_R8G8B8A8_UNORM;
                info.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
                info.subresourceRange.levelCount = 1;
                info.subresourceRange.layerCount = 1;
                info.subresourceRange.baseMipLevel = 0;
                info.subresourceRange.baseArrayLayer = 0;
                vkCreateImageView((VkDevice)ID, &info, nullptr, &view[i]);
                Iinfo[i].imageView = view[i];
                Iinfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                Iinfo[i].sampler = nullptr;
                writes[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                writes[i].pImageInfo = &Iinfo[i];
            }
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = (VkDescriptorSet)sets->ID;
            writes[i].dstBinding = desc[i].binding;
            writes[i].dstArrayElement = 0;
            writes[i].descriptorCount = desc->numDescriptors;
        }
        vkUpdateDescriptorSets((VkDevice)ID, numDescs, writes, 0, nullptr);
        for (int i  = 0; i < 5; i++)
        {
            if (view[i]);
               // vkDestroyImageView((VkDevice)ID, view[i], nullptr);
        }
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
        return flags;
    }
    RESULT Device::CreateTexture(TextureDesc* desc, Texture** texture, Heap* heap, HeapProperties* props, std::uint64_t offset, ResourceType type)
    {
        vTexture* vtexture = new vTexture;
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
        vkCreateImage((VkDevice)ID, &info, nullptr, (VkImage*)&vtexture->ID);
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
}
