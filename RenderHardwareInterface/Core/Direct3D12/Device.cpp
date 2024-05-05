#include "pch.h"
#include "../Device.h"
#include "include/d3d12.h"
#include <d3dcompiler.h>
#include "include\d3dx12\d3dx12.h"
#include "D3D12Specific.h"
#include "Error.h"
#include <iostream>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
namespace RHI
{
    static D3D12_COMMAND_LIST_TYPE D3D12CmdListType(CommandListType type)
    {
        switch (type)
        {
        case(CommandListType::Direct):
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case(CommandListType::Compute):
            return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case(CommandListType::Copy):
            return D3D12_COMMAND_LIST_TYPE_COPY;
        default:
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }
}
extern "C"
{
    RESULT RHI_API RHICreateDevice(RHI::PhysicalDevice* PhysicalDevice, RHI::CommandQueueDesc const* commandQueueInfos, int numCommandQueues, RHI::CommandQueue** commandQueues,Internal_ID instance, RHI::Device** device)
    {
        RHI::D3D12Device* d3d12device = new RHI::D3D12Device;
        RHI::D3D12CommandQueue* d3d12queues = new RHI::D3D12CommandQueue[numCommandQueues];
        D3D12_FEATURE_DATA_D3D12_OPTIONS12 opt;
        opt.EnhancedBarriersSupported = true;
        HRESULT res = D3D12CreateDevice((IUnknown*)PhysicalDevice->ID, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(((ID3D12Device10**)&d3d12device->ID)));
        ((ID3D12Device10*)d3d12device->ID)->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &opt, sizeof(opt));
        *device = d3d12device;
        for (int i = 0; i < numCommandQueues; i++)
        {
            D3D12_COMMAND_QUEUE_DESC desc;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 0;
            desc.Priority = commandQueueInfos[i].Priority < 1.f ? D3D12_COMMAND_QUEUE_PRIORITY_NORMAL : D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
            desc.Type = RHI::D3D12CmdListType(commandQueueInfos[i].CommandListType);
            ((ID3D12Device*)d3d12device->ID)->CreateCommandQueue(&desc, IID_PPV_ARGS(((ID3D12CommandQueue**)&d3d12queues[i].ID)));
            commandQueues[i] = &d3d12queues[i];
        }
        return res;
    }
}
namespace RHI
{
    Default_t Default = {};
    Zero_t Zero = {};
   
    static D3D12_DESCRIPTOR_HEAP_TYPE D3D12DescriptorHeapType(DescriptorType type)
    {
        switch (type)
        {
        case(DescriptorType::RTV):
            return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        case(DescriptorType::DSV):
            return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        case(DescriptorType::ConstantBuffer):
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case(DescriptorType::SampledTexture):
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case(DescriptorType::Sampler):
            return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        default:
            return (D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
        }
    }
    RESULT Device::CreateCommandAllocators(CommandListType type,uint32_t numAllocators,CommandAllocator** pAllocator)
    {
        D3D12CommandAllocator* d3d12allocator = new D3D12CommandAllocator[numAllocators];
        for (uint32_t i = 0; i < numAllocators; i++)
        {
            ((ID3D12Device*)ID)->CreateCommandAllocator(D3D12CmdListType(type), IID_PPV_ARGS(((ID3D12CommandAllocator**)&d3d12allocator[i].ID)));
            pAllocator[i] = &d3d12allocator[i];
        }
        return 0;
    }
    template<> RESULT Device::CreateCommandList<GraphicsCommandList>(CommandListType type, CommandAllocator* allocator, GraphicsCommandList** ppCommandList)
    {
        D3D12GraphicsCommandList* d3d12list = new D3D12GraphicsCommandList;
        RESULT res = ((ID3D12Device*)ID)->CreateCommandList(0, D3D12CmdListType(type), (ID3D12CommandAllocator*)allocator->ID,nullptr,IID_PPV_ARGS(((ID3D12CommandList**)&d3d12list->ID)));
        ((ID3D12GraphicsCommandList*)d3d12list->ID)->Close();
        *ppCommandList = d3d12list;
        return res;
    }
    RESULT Device::CreateDescriptorHeap(DescriptorHeapDesc* desc, DescriptorHeap** descriptorHeap)
    {
        D3D12DescriptorHeap* d3d12heap = new D3D12DescriptorHeap;
        D3D12_DESCRIPTOR_HEAP_DESC dsDesc;
        dsDesc.NumDescriptors = 0;
        for (UINT i = 0; i < desc->numPoolSizes; i++)
        {
            dsDesc.NumDescriptors += desc->poolSizes[i].numDescriptors;
        }
        dsDesc.NodeMask = 0;
        dsDesc.Flags = desc->poolSizes->type == DescriptorType::RTV || desc->poolSizes->type == DescriptorType::DSV ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        dsDesc.Type = D3D12DescriptorHeapType(desc->poolSizes->type);
        ((ID3D12Device*)ID)->CreateDescriptorHeap(&dsDesc, IID_PPV_ARGS(((ID3D12DescriptorHeap**)&d3d12heap->ID)));
        *descriptorHeap = d3d12heap;
        return 0;
    }
    RESULT Device::CreateDynamicDescriptor(DescriptorHeap* heap, DynamicDescriptor** descriptor, DescriptorType type, ShaderStage stage, RHI::Buffer* buffer, uint32_t offset, uint32_t size)
    {
        D3D12DynamicDescriptor* d3d12desc = new D3D12DynamicDescriptor;
        d3d12desc->address = ((ID3D12Resource*)buffer->ID)->GetGPUVirtualAddress();
        d3d12desc->address += offset;
        *descriptor = d3d12desc;
        return 0;
    }
    RESULT Device::CreateDescriptorSets(DescriptorHeap* heap, std::uint32_t numDescriptorSets, DescriptorSetLayout* layouts, DescriptorSet** pSets)
    {
        D3D12DescriptorHeap* Heap = (D3D12DescriptorHeap*)heap;
        D3D12_CPU_DESCRIPTOR_HANDLE handle = { ((ID3D12DescriptorHeap*)heap->ID)->GetCPUDescriptorHandleForHeapStart().ptr + Heap->offset };
        D3D12_GPU_DESCRIPTOR_HANDLE ghandle = { ((ID3D12DescriptorHeap*)heap->ID)->GetGPUDescriptorHandleForHeapStart().ptr + Heap->offset};
        SIZE_T offset = 0;
        D3D12DescriptorSet* sets = new D3D12DescriptorSet[numDescriptorSets];
        for (UINT i = 0; i < numDescriptorSets; i++)
        {
            sets[i].start.val = handle.ptr + offset;
            sets[i].gpu_handle = ghandle.ptr + offset;
            pSets[i] = &sets[i];
            offset += ((D3D12DescriptorSetLayout*)layouts)[i].numDescriptors * ((ID3D12Device*)ID)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        Heap->offset += offset;
        return 0;
    }
    RESULT Device::UpdateDescriptorSets(std::uint32_t numDescs, DescriptorSetUpdateDesc* desc, DescriptorSet* sets)
    {
        for (UINT i = 0; i < numDescs; i++)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle;
            handle.ptr = ((D3D12DescriptorSet*)sets)[0].start.val + (desc[i].binding * ((ID3D12Device*)ID)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
            if (desc[i].type == DescriptorType::ConstantBuffer)
            {
                D3D12_CONSTANT_BUFFER_VIEW_DESC cbdesc;
                cbdesc.BufferLocation = ((ID3D12Resource*)desc[i].bufferInfos->buffer->ID)->GetGPUVirtualAddress() + (desc[i].bufferInfos->offset);
                cbdesc.SizeInBytes = desc[i].bufferInfos->range;
                ((ID3D12Device*)ID)->CreateConstantBufferView(&cbdesc, handle);
            }
            else if (desc[i].type == DescriptorType::StructuredBuffer)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                //todo
            }
            else if (desc[i].type == DescriptorType::SampledTexture)
            {
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                srvDesc.Texture2D.MipLevels = 1;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.PlaneSlice = 0;
                srvDesc.Texture2D.ResourceMinLODClamp = 0;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                ((ID3D12Device*)ID)->CreateShaderResourceView((ID3D12Resource*)desc[i].textureInfos->texture->ID, &srvDesc, handle);
            }
        }
        return 0;
    }
    RESULT Device::CreateRenderTargetView(Texture* texture, RenderTargetViewDesc* desc, CPU_HANDLE heapHandle)
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = (DXGI_FORMAT)desc->format;
        rtvDesc.ViewDimension = desc->TextureArray? D3D12_RTV_DIMENSION_TEXTURE1DARRAY: D3D12_RTV_DIMENSION_TEXTURE2D;
        if (desc->TextureArray)
        {
            rtvDesc.Texture2DArray.ArraySize = 1;
            rtvDesc.Texture2DArray.FirstArraySlice = desc->arraySlice;
            rtvDesc.Texture2DArray.MipSlice = desc->textureMipSlice;
            rtvDesc.Texture2DArray.PlaneSlice = 0;
        }
        else
        {
            rtvDesc.Texture2D.MipSlice = desc->textureMipSlice;
            rtvDesc.Texture2D.PlaneSlice = 0;//??todo
        }
        ((ID3D12Device*)ID)->CreateRenderTargetView((ID3D12Resource*)texture->ID, &rtvDesc, {heapHandle.val}); //todo
        return 0;
    }
    RESULT Device::CreateDepthStencilView(Texture* texture, DepthStencilViewDesc* desc, CPU_HANDLE heapHandle)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = (DXGI_FORMAT)desc->format;
        if (desc->TextureArray)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.ArraySize = 1;
            dsvDesc.Texture2DArray.FirstArraySlice = desc->arraySlice;
            dsvDesc.Texture2DArray.MipSlice = desc->textureMipSlice;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = desc->textureMipSlice;
        }
        ((ID3D12Device*)ID)->CreateDepthStencilView((ID3D12Resource*)texture->ID, &dsvDesc, {heapHandle.val});//todo
        return RESULT();
    }
    std::uint32_t Device::GetDescriptorHeapIncrementSize(DescriptorType type)
    {
        return ((ID3D12Device*)ID)->GetDescriptorHandleIncrementSize(D3D12DescriptorHeapType(type));
    }
    RESULT Device::GetSwapChainImage(SwapChain* swapchain, std::uint32_t index, Texture** texture)
    {
        (*texture) = new D3D12Texture;
        return ((IDXGISwapChain*)swapchain->ID)->GetBuffer(index, IID_PPV_ARGS((ID3D12Resource**)&(*texture)->ID));
    }
    RESULT Device::CreateFence(Fence** fence, std::uint64_t val)
    {
        D3D12Fence* d3d12fence = new D3D12Fence;
        ((ID3D12Device*)ID)->CreateFence(val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS((ID3D12Fence**)&d3d12fence->ID));
        *fence = d3d12fence;
        return 0;
    }
    static void ReadFileToBlob(ID3DBlob** blob, const char* filename)
    {
        wchar_t buffer[256];
        mbstowcs(buffer, filename, 256);
        wcscat(buffer, L".cso");
        D3DReadFileToBlob(buffer, blob);
    }
    D3D12_FILL_MODE D3DFillMode(RHI::FillMode mode)
    {
        switch (mode)
        {
        case RHI::FillMode::Solid: return D3D12_FILL_MODE_SOLID;
            break;
        case RHI::FillMode::Wireframe:return D3D12_FILL_MODE_WIREFRAME;
            break;
        default:
            break;
        }
    }
    D3D12_CULL_MODE D3DCullMode(CullMode mode)
    {
        switch (mode)
        {
        case RHI::CullMode::None: return D3D12_CULL_MODE_NONE;
            break;
        case RHI::CullMode::Front: return D3D12_CULL_MODE_FRONT;
            break;
        case RHI::CullMode::Back: return D3D12_CULL_MODE_BACK;
            break;
        default:
            break;
        }
    }
    UINT D3DSampleCount(SampleMode mode)
    {
        switch (mode)
        {
        case RHI::SampleMode::x2: return 2;
            break;
        case RHI::SampleMode::x4: return 4;
            break;
        case RHI::SampleMode::x8: return 8;
            break;
        default:
            break;
        }
    }
    D3D12_CONSERVATIVE_RASTERIZATION_MODE D3DConservativeRaster(bool raster)
    {
        return raster ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    }
    RESULT Device::CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject** pPSO)
    {
        D3D12PipelineStateObject* d3d12pso = new D3D12PipelineStateObject;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC PSOdesc{};
        ID3DBlob* blob[5] = {};
        if(desc->VS) 
        {
            ReadFileToBlob(&blob[0], desc->VS);
            PSOdesc.VS = {blob[0]->GetBufferPointer(), blob[0]->GetBufferSize()};
        }
        if (desc->PS)
        {
            ReadFileToBlob(&blob[1], desc->PS);
            PSOdesc.PS = { blob[1]->GetBufferPointer(), blob[1]->GetBufferSize() };
        }
        if (desc->GS)
        {
            ReadFileToBlob(&blob[2], desc->GS);
            PSOdesc.GS = { blob[2]->GetBufferPointer(), blob[2]->GetBufferSize() };
        }
        if (desc->HS)
        {
            ReadFileToBlob(&blob[3], desc->HS);
            PSOdesc.HS = { blob[3]->GetBufferPointer(), blob[3]->GetBufferSize() };
        }
        if (desc->DS)
        {
            ReadFileToBlob(&blob[4], desc->DS);
            PSOdesc.DS = { blob[4]->GetBufferPointer(), blob[4]->GetBufferSize() };
        }
        D3D12_INPUT_ELEMENT_DESC ied[5];
        for (int i = 0; i < desc->numInputElements; i++)
        {
            GFX_ASSERT(desc->inputElements[i].inputSlot < desc->numInputBindings);
            ied[i].SemanticName = "SEM";
            ied[i].InputSlot = desc->inputElements[i].inputSlot;
            ied[i].AlignedByteOffset = desc->inputElements[i].alignedByteOffset;
            ied[i].Format = (DXGI_FORMAT)desc->inputElements[i].format;
            ied[i].InputSlotClass = desc->inputBindings[desc->inputElements[i].inputSlot].inputRate == InputRate::Vertex ? D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA : D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            ied[i].SemanticIndex = desc->inputElements[i].location;
            ied[i].InstanceDataStepRate = 0;
        }
        for (int i = 0; i < desc->numInputBindings; i++)
        {
            d3d12pso->strides.push_back(desc->inputBindings[i].stride);
        }
        PSOdesc.InputLayout = {ied, desc->numInputElements};
        PSOdesc.pRootSignature = (ID3D12RootSignature*)desc->rootSig->ID;
        PSOdesc.RasterizerState = CD3DX12_RASTERIZER_DESC(
            D3DFillMode(desc->rasterizerMode.fillMode), 
            D3DCullMode(desc->rasterizerMode.cullMode), 
            false, 
            desc->rasterizerMode.depthBias, 
            desc->rasterizerMode.depthBiasClamp,
            desc->rasterizerMode.slopeScaledDepthBias,
            desc->rasterizerMode.depthClipEnable,
            desc->rasterizerMode.multisampleEnable,
            desc->rasterizerMode.AntialiasedLineEnable,
            0,//D3DSampleCount(desc->sampleCount),
            D3DConservativeRaster(desc->rasterizerMode.conservativeRaster));
        PSOdesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        PSOdesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        PSOdesc.DepthStencilState.DepthEnable = desc->depthStencilMode.DepthEnable;
        PSOdesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //todo
        PSOdesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        PSOdesc.SampleMask = UINT_MAX;
        PSOdesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        PSOdesc.NumRenderTargets = 1;
        PSOdesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
        PSOdesc.SampleDesc.Count = 1;
        PSOdesc.SampleDesc.Quality = 0;
        PSOdesc.DSVFormat = (DXGI_FORMAT)desc->DSVFormat;
        HRESULT res = ((ID3D12Device*)ID)->CreateGraphicsPipelineState(&PSOdesc, IID_PPV_ARGS((ID3D12PipelineState**)&d3d12pso->ID));
        *pPSO = d3d12pso;
        for (int i = 0; i < 5; i++) { if (blob[i])blob[i]->Release(); };
        return 0;
    }
    D3D12_RESOURCE_DIMENSION ResourceDimension(RHI::TextureType type)
    {
        switch (type)
        {
        case RHI::TextureType::Texture1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            break;
        case RHI::TextureType::Texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            break;
        case RHI::TextureType::Texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
            break;
        default:
            break;
        }
    }
    D3D12_RESOURCE_FLAGS ResourceFlags(RHI::TextureUsage usage)
    {
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if ((usage & RHI::TextureUsage::DepthStencilAttachment) != RHI::TextureUsage::None)
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        if ((usage & RHI::TextureUsage::ColorAttachment) != RHI::TextureUsage::None)
        {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if ((usage & RHI::TextureUsage::SampledImage) == RHI::TextureUsage::None)
        {
            flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
        return flags;
    }
    D3D12_HEAP_PROPERTIES D3D12HeapProps(RHI::HeapProperties* props)
    {
        D3D12_HEAP_PROPERTIES hp;
        if (props->type == HeapType::Custom)
        {
            hp.Type = D3D12_HEAP_TYPE_CUSTOM;
            if (props->memoryLevel == MemoryLevel::DedicatedRAM) hp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            if (props->memoryLevel == MemoryLevel::SharedRAM) hp.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
            if (props->memoryLevel == MemoryLevel::Unknown) hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            if (props->pageProperty == CPUPageProperty::WriteCombined) hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            if (props->pageProperty == CPUPageProperty::WriteBack) hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
            if (props->pageProperty == CPUPageProperty::Any) hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            if (props->pageProperty == CPUPageProperty::NonVisible) hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
        }
        else
        {
            hp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        }
        if (props->type == HeapType::Default) hp.Type = D3D12_HEAP_TYPE_DEFAULT;
        if (props->type == HeapType::Readback) hp.Type = D3D12_HEAP_TYPE_READBACK;
        if (props->type == HeapType::Upload) hp.Type = D3D12_HEAP_TYPE_UPLOAD;
        hp.CreationNodeMask = hp.VisibleNodeMask = 0;
        return hp;
    }
    RESULT Device::CreateTexture(TextureDesc* desc, Texture** texture, Heap* heap, HeapProperties* props, AutomaticAllocationInfo* autoInfo,std::uint64_t offset, ResourceType type)
    {
        D3D12Texture* d3d12texture = new D3D12Texture;
        D3D12_RESOURCE_DESC td{};
        td.Dimension = ResourceDimension(desc->type);
        td.Format = (DXGI_FORMAT)desc->format;
        td.MipLevels = desc->mipLevels;
        td.Alignment = 0;
        td.DepthOrArraySize = desc->depthOrArraySize;
        td.Height = desc->height;
        td.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        td.SampleDesc.Count = desc->sampleCount;
        td.Width = desc->width;
        td.Flags = ResourceFlags(desc->usage);
        D3D12_CLEAR_VALUE* cval_ptr = nullptr;
        D3D12_CLEAR_VALUE cval;
        if (desc->optimizedClearValue != nullptr)
        {
            cval.Format = (DXGI_FORMAT)desc->format;
            cval.Color[0] = desc->optimizedClearValue->clearColor.r;
            cval.Color[1] = desc->optimizedClearValue->clearColor.g;
            cval.Color[2] = desc->optimizedClearValue->clearColor.b;
            cval.Color[3] = desc->optimizedClearValue->clearColor.a;
            cval_ptr = &cval;
        }
        if (type == ResourceType::Automatic)
        {
            D3D12_HEAP_PROPERTIES props;
            props.CPUPageProperty = autoInfo->access_mode == AutomaticAllocationCPUAccessMode::None ? D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            props.CreationNodeMask = 0;
            props.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            props.Type = D3D12_HEAP_TYPE_CUSTOM;
            props.VisibleNodeMask = 0;
            ((ID3D12Device*)ID)->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &td, D3D12_RESOURCE_STATE_COMMON, cval_ptr, IID_PPV_ARGS((ID3D12Resource**)&d3d12texture->ID));
        }
        if (type == ResourceType::Commited)
        {
            D3D12_HEAP_PROPERTIES hProp = D3D12HeapProps(props);
            ((ID3D12Device*)ID)->CreateCommittedResource(&hProp, D3D12_HEAP_FLAG_NONE, &td, D3D12_RESOURCE_STATE_COMMON, cval_ptr, IID_PPV_ARGS((ID3D12Resource**)&d3d12texture->ID));
        }
        if (type == ResourceType::Placed)
            ((ID3D12Device*)ID)->CreatePlacedResource((ID3D12Heap*)heap->ID, offset, &td, D3D12_RESOURCE_STATE_COMMON, cval_ptr, IID_PPV_ARGS((ID3D12Resource**)&d3d12texture->ID));
        *texture = d3d12texture;
        return RESULT();
    }
    RESULT Device::CreateBuffer(BufferDesc* desc, Buffer** buffer, Heap* heap, HeapProperties* props,AutomaticAllocationInfo* autoInfo,std::uint64_t offset,ResourceType type)
    {
        D3D12Buffer* d3d12buffer = new D3D12Buffer;
        D3D12_RESOURCE_DESC bd{};
        bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bd.Format = DXGI_FORMAT_UNKNOWN;
        bd.MipLevels = 1;
        bd.DepthOrArraySize = 1;
        bd.Alignment = 0;// D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        bd.Height = 1;
        bd.SampleDesc.Count = 1;
        bd.SampleDesc.Quality = 0;
        bd.Width = desc->size;
        bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        //todo D3D12 MA
        if (type == ResourceType::Automatic)
        {
            D3D12_HEAP_PROPERTIES props;
            props.CPUPageProperty = autoInfo->access_mode == AutomaticAllocationCPUAccessMode::None ? D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE : D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            props.CreationNodeMask = 0;
            props.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            props.Type = D3D12_HEAP_TYPE_CUSTOM;
            props.VisibleNodeMask = 0;
            ((ID3D12Device*)ID)->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS((ID3D12Resource**)&d3d12buffer->ID));
        }
        if (type == ResourceType::Commited)
        {
            D3D12_HEAP_PROPERTIES hProp = D3D12HeapProps(props);
            ((ID3D12Device*)ID)->CreateCommittedResource(&hProp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS((ID3D12Resource**)&d3d12buffer->ID));
        }
        if (type == ResourceType::Placed)
            ((ID3D12Device*)ID)->CreatePlacedResource((ID3D12Heap*)heap->ID, offset, &bd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS((ID3D12Resource**)&d3d12buffer->ID));
        *buffer = d3d12buffer;
        return RESULT();
    }
    RESULT Device::GetBufferMemoryRequirements(BufferDesc* desc, MemoryReqirements* requirements)
    {
        D3D12_RESOURCE_DESC bd{};
        bd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bd.Format = DXGI_FORMAT_UNKNOWN;
        bd.MipLevels = 1;
        bd.DepthOrArraySize = 1;
        bd.Alignment = 0;
        bd.Height = 1;
        bd.SampleDesc.Count = 1;
        bd.SampleDesc.Quality = 0;
        bd.Width = desc->size;
        bd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        D3D12_RESOURCE_ALLOCATION_INFO info = ((ID3D12Device*)ID)->GetResourceAllocationInfo(0, 1, &bd);
        requirements->size = info.SizeInBytes;
        requirements->alignment = info.Alignment;
        return 0;
    }
    RESULT Device::GetTextureMemoryRequirements(TextureDesc* desc, MemoryReqirements* requirements)
    {
        D3D12_RESOURCE_DESC td{};
        td.Dimension = ResourceDimension(desc->type);
        td.Format = (DXGI_FORMAT)desc->format;
        td.MipLevels = desc->mipLevels;
        td.Alignment = 0;
        td.DepthOrArraySize = desc->depthOrArraySize;
        td.Height = desc->height;
        td.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        td.SampleDesc.Count = desc->sampleCount;
        td.Width = desc->width;
        D3D12_RESOURCE_ALLOCATION_INFO info = ((ID3D12Device*)ID)->GetResourceAllocationInfo(0, 1, &td);
        requirements->size = info.SizeInBytes;
        requirements->alignment = info.Alignment;
        return 0;
    }
    RESULT Device::CreateHeap(HeapDesc* desc, Heap** heap, bool* usedFallback)
    {
        D3D12Heap* d3d12Heap = new D3D12Heap;
        D3D12_HEAP_DESC hd{};
        hd.Alignment = 0;
        hd.SizeInBytes = desc->size;
        hd.Properties.CreationNodeMask = hd.Properties.VisibleNodeMask = 0;
        hd.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
        if (desc->props.type == HeapType::Custom)
        {
            hd.Properties.Type = D3D12_HEAP_TYPE_CUSTOM;
            if (desc->props.memoryLevel == MemoryLevel::DedicatedRAM) hd.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
            if (desc->props.memoryLevel == MemoryLevel::SharedRAM) hd.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1;
            if (desc->props.memoryLevel == MemoryLevel::Unknown) hd.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            if (desc->props.pageProperty == CPUPageProperty::WriteCombined) hd.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            if (desc->props.pageProperty == CPUPageProperty::WriteBack) hd.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
            if (desc->props.pageProperty == CPUPageProperty::Any) hd.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
            if (desc->props.pageProperty == CPUPageProperty::NonVisible) hd.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
        }
        else
        {
            hd.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            hd.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        }
        if (desc->props.type == HeapType::Default) hd.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        if (desc->props.type == HeapType::Readback) hd.Properties.Type = D3D12_HEAP_TYPE_READBACK;
        if (desc->props.type == HeapType::Upload) hd.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        if (((ID3D12Device*)ID)->CreateHeap(&hd, IID_PPV_ARGS((ID3D12Heap**)&d3d12Heap->ID)) != 0)
        {
            if (desc->props.FallbackType == HeapType::Default)  hd.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
            if (desc->props.FallbackType == HeapType::Readback) hd.Properties.Type = D3D12_HEAP_TYPE_READBACK;
            if (desc->props.FallbackType == HeapType::Upload)   hd.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            return ((ID3D12Device*)ID)->CreateHeap(&hd, IID_PPV_ARGS((ID3D12Heap**)&d3d12Heap->ID));
        }
        *heap = d3d12Heap;
        return 0;
    }
    D3D12_DESCRIPTOR_RANGE_TYPE RangeType(DescriptorType type)
    {
        switch (type)
        {
        case RHI::DescriptorType::SampledTexture: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            break;
        case RHI::DescriptorType::ConstantBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            break;
        case RHI::DescriptorType::StructuredBuffer:return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            break;
        case RHI::DescriptorType::ConstantBufferDynamic: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            break;
        case RHI::DescriptorType::StructuredBufferDynamic:return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            break;
        case RHI::DescriptorType::Sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            break;
        default:
            break;
        }
    }
    RESULT Device::CreateRootSignature(RootSignatureDesc* desc, RootSignature** rootSignature,DescriptorSetLayout** pSetLayouts)
    {
        D3D12RootSignature* d3d12rootsig = new D3D12RootSignature;
        D3D12_ROOT_PARAMETER params[5]{};
        D3D12_ROOT_SIGNATURE_DESC rsDesc;
        D3D12_DESCRIPTOR_RANGE range[20];
        UINT rangeIndex = 0;
        D3D12DescriptorSetLayout* dSetLayout = new D3D12DescriptorSetLayout[desc->numRootParameters];
        for (UINT i = 0; i < desc->numRootParameters; i++)
        {
            if (desc->rootParameters[i].type == RootParameterType::DynamicDescriptor)
            {
                GFX_ASSERT(desc->rootParameters[i].dynamicDescriptor.type == DescriptorType::ConstantBufferDynamic ||
                    desc->rootParameters[i].dynamicDescriptor.type == DescriptorType::StructuredBufferDynamic)
                params[i].ParameterType = desc->rootParameters[i].dynamicDescriptor.type == DescriptorType::ConstantBufferDynamic ? D3D12_ROOT_PARAMETER_TYPE_CBV : D3D12_ROOT_PARAMETER_TYPE_SRV;
                params[i].Descriptor.RegisterSpace = desc->rootParameters[i].dynamicDescriptor.setIndex;
                params[i].Descriptor.ShaderRegister = 0;
                params[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                continue;
            }
            params[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[i].DescriptorTable.NumDescriptorRanges = desc->rootParameters[i].descriptorTable.numDescriptorRanges;
            dSetLayout[i].numBindings = desc->rootParameters[i].descriptorTable.numDescriptorRanges;
            dSetLayout[i].numDescriptors = 0;
            params[i].DescriptorTable.pDescriptorRanges = &range[rangeIndex];
            for (UINT j = 0; j < desc->rootParameters[i].descriptorTable.numDescriptorRanges; j++)
            {
                range[rangeIndex].BaseShaderRegister = desc->rootParameters[i].descriptorTable.ranges[j].BaseShaderRegister;
                range[rangeIndex].RegisterSpace = desc->rootParameters[i].descriptorTable.setIndex;
                range[rangeIndex].NumDescriptors = desc->rootParameters[i].descriptorTable.ranges[j].numDescriptors;
                range[rangeIndex].RangeType = RangeType(desc->rootParameters[i].descriptorTable.ranges[j].type);
                range[rangeIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                dSetLayout[i].numDescriptors += range[j].NumDescriptors;
                rangeIndex++;
            }
            pSetLayouts[i] = &dSetLayout[i];
            params[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }
        rsDesc = CD3DX12_ROOT_SIGNATURE_DESC(desc->numRootParameters, params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ID3DBlob* serializedRootSig = nullptr;
        ID3DBlob* errorBlob = nullptr;
        D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &serializedRootSig);
        if (errorBlob != nullptr)
        {
            ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        RESULT res = ((ID3D12Device*)ID)->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS((ID3D12RootSignature**)&d3d12rootsig->ID));
        serializedRootSig->Release();
        *rootSignature = d3d12rootsig;
        return res;
    }
    
    Device::~Device()
    {
        ID3D12DebugDevice* device;
        ((ID3D12Device*)ID)->QueryInterface(&device);
        device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY);
    }
}
