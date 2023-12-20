#include "pch.h"
#include "../Device.h"
#include "include/d3d12.h"
#include <d3dcompiler.h>
#include "include\d3dx12\d3dx12.h"
#include "Error.h"
#include <iostream>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")
namespace RHI
{
    Default_t Default = {};
    Zero_t Zero = {};
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
    static D3D12_DESCRIPTOR_HEAP_TYPE D3D12DescriptorHeapType(DescriptorHeapType type)
    {
        switch (type)
        {
        case(DescriptorHeapType::RTV):
            return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        case(DescriptorHeapType::DSV):
            return D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        case(DescriptorHeapType::SRV_CBV_UAV):
            return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        case(DescriptorHeapType::SAMPLER):
            return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        default:
            return (D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES);
        }
    }
    RESULT Device::Create(PhysicalDevice PhysicalDevice, CommandQueueDesc const* commandQueueInfos, int numCommandQueues, CommandQueue* commandQueues, Device* device)
    {
        D3D12_FEATURE_DATA_D3D12_OPTIONS12 opt;
        opt.EnhancedBarriersSupported = true;
        HRESULT res = D3D12CreateDevice((IUnknown*)PhysicalDevice.ID, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(((ID3D12Device10**)&device->ID)));
        ((ID3D12Device10*)device->ID)->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &opt, sizeof(opt));
        for (int i = 0; i < numCommandQueues; i++)
        {
            D3D12_COMMAND_QUEUE_DESC desc;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 0;
            desc.Priority = commandQueueInfos[i].Priority < 1.f ? D3D12_COMMAND_QUEUE_PRIORITY_NORMAL : D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
            desc.Type = D3D12CmdListType(commandQueueInfos[i].CommandListType);
            ((ID3D12Device*)device->ID)->CreateCommandQueue(&desc, IID_PPV_ARGS(((ID3D12CommandQueue**)&commandQueues[i].ID)));
        }
        return res;
    }
    RESULT Device::CreateCommandAllocator(CommandListType type,CommandAllocator* pAllocator)
    {
        return ((ID3D12Device*)ID)->CreateCommandAllocator(D3D12CmdListType(type), IID_PPV_ARGS(((ID3D12CommandAllocator**)&pAllocator->ID)));
    }
    RESULT Device::CreateCommandList(CommandListType type, CommandAllocator allocator, CommandList* pCommandList)
    {
        RESULT res = ((ID3D12Device*)ID)->CreateCommandList(0, D3D12CmdListType(type), (ID3D12CommandAllocator*)allocator.ID,nullptr,IID_PPV_ARGS(((ID3D12CommandList**)&pCommandList->ID)));
        ((ID3D12GraphicsCommandList*)pCommandList->ID)->Close();
        return res;
    }
    RESULT Device::CreateDescriptorHeap(DescriptorHeapDesc* desc, DescriptorHeap* descriptorHeap)
    {

        D3D12_DESCRIPTOR_HEAP_DESC dsDesc;
        dsDesc.NumDescriptors = 0;
        for (UINT i = 0; i < desc->numPoolSizes; i++)
            dsDesc.NumDescriptors += desc->poolSizes[i].numDescriptors;
        dsDesc.NodeMask = 0;
        dsDesc.Flags = desc->poolSizes->type == DescriptorType::RTV || desc->poolSizes->type == DescriptorType::DSV ? D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        dsDesc.Type = D3D12DescriptorHeapType(desc->poolSizes->type);
        return ((ID3D12Device*)ID)->CreateDescriptorHeap(&dsDesc, IID_PPV_ARGS(((ID3D12DescriptorHeap**)&descriptorHeap->ID)));
    }
    RESULT Device::CreateDescriptorSets(DescriptorHeap heap, std::uint32_t numDescriptorSets, DescriptorSetLayout* layouts, DescriptorSet* pSets)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = ((ID3D12DescriptorHeap*)heap.ID)->GetCPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE ghandle = ((ID3D12DescriptorHeap*)heap.ID)->GetGPUDescriptorHandleForHeapStart();
        SIZE_T offset = 0;
        
        for (UINT i = 0; i < numDescriptorSets; i++)
        {
            pSets[i].start.val = handle.ptr + offset;
            pSets[i].gpu_handle = ghandle.ptr + offset;
            offset += layouts[i].numDescriptors * ((ID3D12Device*)ID)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }
        return 0;
    }
    RESULT Device::UpdateDescriptorSets(std::uint32_t numDescs, DescriptorSetUpdateDesc* desc, DescriptorSet* sets)
    {
        for (UINT i = 0; i < numDescs; i++)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle;
            handle.ptr = sets[0].start.val + (desc[i].binding * ((ID3D12Device*)ID)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbdesc;
            cbdesc.BufferLocation = ((ID3D12Resource*)desc[i].bufferInfos->buffer.ID)->GetGPUVirtualAddress() + (desc[i].bufferInfos->offset);
            cbdesc.SizeInBytes = desc[i].bufferInfos->range;
            ((ID3D12Device*)ID)->CreateConstantBufferView(&cbdesc, handle);
        }
        return 0;
    }
    RESULT Device::CreateRenderTargetView(Texture texture, RenderTargetViewDesc* desc, CPU_HANDLE heapHandle)
    {
        ((ID3D12Device*)ID)->CreateRenderTargetView((ID3D12Resource*)texture.ID, nullptr, {heapHandle.val});
        return 0;
    }
    std::uint32_t Device::GetDescriptorHeapIncrementSize(DescriptorHeapType type)
    {
        return ((ID3D12Device*)ID)->GetDescriptorHandleIncrementSize(D3D12DescriptorHeapType(type));
    }
    RESULT Device::GetSwapChainImage(SwapChain swapchain, std::uint32_t index, Texture* texture)
    {
        return ((IDXGISwapChain*)swapchain.ID)->GetBuffer(index, IID_PPV_ARGS((ID3D12Resource**)&texture->ID));
    }
    RESULT Device::CreateFence(Fence* fence, std::uint64_t val)
    {
        return ((ID3D12Device*)ID)->CreateFence(val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS((ID3D12Fence**)&fence->ID));
    }
    static void ReadFileToBlob(ID3DBlob** blob, const char* filename)
    {
        wchar_t buffer[256];
        mbstowcs(buffer, filename, 256);
        wcscat(buffer, L".cso");
        D3DReadFileToBlob(buffer, blob);
    }
    RESULT Device::CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject* pPSO)
    {
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
            pPSO->strides.push_back(desc->inputBindings[i].stride);
        }
        PSOdesc.InputLayout = {ied, desc->numInputElements};
        PSOdesc.pRootSignature = (ID3D12RootSignature*)desc->rootSig.ID;
        PSOdesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        PSOdesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        PSOdesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        PSOdesc.DepthStencilState = { NULL };// CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        PSOdesc.SampleMask = UINT_MAX;
        PSOdesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        PSOdesc.NumRenderTargets = 1;
        PSOdesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
        PSOdesc.SampleDesc.Count = 1;
        PSOdesc.SampleDesc.Quality = 0;
        PSOdesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
        HRESULT res = ((ID3D12Device*)ID)->CreateGraphicsPipelineState(&PSOdesc, IID_PPV_ARGS((ID3D12PipelineState**)&pPSO->ID));
        
        for (int i = 0; i < 5; i++) { if (blob[i])blob[i]->Release(); };
        return 0;
    }
    RESULT Device::CreateBuffer(BufferDesc* desc, Buffer* buffer, Heap* heap, std::uint64_t offset,ResourceType type)
    {
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
        CD3DX12_HEAP_PROPERTIES hProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        if(type == ResourceType::Commited)
            ((ID3D12Device*)ID)->CreateCommittedResource(&hProp, D3D12_HEAP_FLAG_NONE, &bd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS((ID3D12Resource**)&buffer->ID));
        if (type == ResourceType::Placed)
            ((ID3D12Device*)ID)->CreatePlacedResource((ID3D12Heap*)heap->ID, offset, &bd, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS((ID3D12Resource**)&buffer->ID));
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
    RESULT Device::CreateHeap(HeapDesc* desc, Heap* heap, bool* usedFallback)
    {
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
        if (((ID3D12Device*)ID)->CreateHeap(&hd, IID_PPV_ARGS((ID3D12Heap**)&heap->ID)) != 0)
        {
            if (desc->props.FallbackType == HeapType::Default)  hd.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
            if (desc->props.FallbackType == HeapType::Readback) hd.Properties.Type = D3D12_HEAP_TYPE_READBACK;
            if (desc->props.FallbackType == HeapType::Upload)   hd.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
            return ((ID3D12Device*)ID)->CreateHeap(&hd, IID_PPV_ARGS((ID3D12Heap**)&heap->ID));
        }
        return 0;
    }
    RESULT Device::CreateRootSignature(RootSignatureDesc* desc, RootSignature* rootSignature,_Out_ DescriptorSetLayout* pSetLayouts)
    {
        D3D12_ROOT_PARAMETER params[5]{};
        D3D12_ROOT_SIGNATURE_DESC rsDesc;
        D3D12_DESCRIPTOR_RANGE range[20];
        UINT rangeIndex = 0;
        for (UINT i = 0; i < desc->numRootParameters; i++)
        {
            params[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[i].DescriptorTable.NumDescriptorRanges = desc->rootParameters[i].descriptorTable.numDescriptorRanges;
            pSetLayouts[i].numBindings = desc->rootParameters[i].descriptorTable.numDescriptorRanges;
            pSetLayouts[i].numDescriptors = 0;
            params[i].DescriptorTable.pDescriptorRanges = &range[rangeIndex];
            for (UINT j = 0; j < desc->rootParameters[i].descriptorTable.numDescriptorRanges; j++)
            {
                range[rangeIndex].BaseShaderRegister = desc->rootParameters[i].descriptorTable.ranges[j].BaseShaderRegister;
                range[rangeIndex].RegisterSpace = i;
                range[rangeIndex].NumDescriptors = desc->rootParameters[i].descriptorTable.ranges[j].numDescriptors;
                range[rangeIndex].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                range[rangeIndex].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
                pSetLayouts[i].numDescriptors += range[j].NumDescriptors;
                rangeIndex++;
            }
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
        RESULT res = ((ID3D12Device*)ID)->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS((ID3D12RootSignature**)&rootSignature->ID));
        serializedRootSig->Release();
        return res;
    }
}
