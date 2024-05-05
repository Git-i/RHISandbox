#include "pch.h"
#include "../CommandList.h"
#include "include/d3d12.h"
#include "include\d3dx12\d3dx12.h"
#include "D3D12Specific.h"
namespace RHI
{
    static std::uint32_t* strides;
    D3D12_TEXTURE_BARRIER RHI_API ConvertToDeviceFormat(TextureMemoryBarrier* desc)
    {
        D3D12_TEXTURE_BARRIER barr = {};
        barr.AccessBefore = D3DAcessMode(desc->AccessFlagsBefore);
        barr.AccessAfter = D3DAcessMode(desc->AccessFlagsAfter);
        barr.LayoutBefore = D3DBarrierLayout(desc->oldLayout);
        barr.LayoutAfter = D3DBarrierLayout(desc->newLayout);
        barr.pResource = (ID3D12Resource*)desc->texture->ID;
        D3D12_BARRIER_SUBRESOURCE_RANGE range;
        range.FirstArraySlice = desc->subresourceRange.FirstArraySlice;
        range.NumPlanes = 1;
        range.IndexOrFirstMipLevel = desc->subresourceRange.IndexOrFirstMipLevel;
        range.NumMipLevels = desc->subresourceRange.NumMipLevels;
        range.NumArraySlices = desc->subresourceRange.NumArraySlices;
        UINT firstPlane = 0;
        UINT numPlanes = 0;
        if ((UINT)desc->subresourceRange.imageAspect & (UINT)RHI::Aspect::STENCIL_BIT)
        {
            firstPlane = 1;
        }
        if ((UINT)desc->subresourceRange.imageAspect & (UINT)RHI::Aspect::PLANE_1_BIT)
        {
            firstPlane = 1;
        }
        if ((UINT)desc->subresourceRange.imageAspect & (UINT)RHI::Aspect::PLANE_2_BIT)
        {
            firstPlane = 2;
        }
        range.FirstPlane = firstPlane;

        return barr;
    }
    D3D12_BARRIER_ACCESS D3DAcessMode(ResourceAcessFlags flags)
    {
        switch (flags)
        {
        case RHI::ResourceAcessFlags::INDEX_BUFFER_READ: return D3D12_BARRIER_ACCESS_INDEX_BUFFER;
            break;
        case RHI::ResourceAcessFlags::VERTEX_BUFFER_READ: return D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
            break;
        case RHI::ResourceAcessFlags::CONSTANT_BUFFER_READ: return D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
            break;
        case RHI::ResourceAcessFlags::INPUT_ATTACHMENT_READ: return D3D12_BARRIER_ACCESS_RENDER_TARGET | D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
            break;
        case RHI::ResourceAcessFlags::SHADER_READ: return D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
            break;
        case RHI::ResourceAcessFlags::SHADER_WRITE: return D3D12_BARRIER_ACCESS_SHADER_RESOURCE; // I DONT KNOW
            break;
        case RHI::ResourceAcessFlags::COLOR_ATTACHMENT_READ: return D3D12_BARRIER_ACCESS_COPY_SOURCE;
            break;
        case RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE: return D3D12_BARRIER_ACCESS_RENDER_TARGET;
            break;
        case RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_READ: return D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
            break;
        case RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE: return D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
            break;
        case RHI::ResourceAcessFlags::TRANSFER_READ: return D3D12_BARRIER_ACCESS_COPY_SOURCE;
            break;
        case RHI::ResourceAcessFlags::TRANSFER_WRITE: return D3D12_BARRIER_ACCESS_COPY_DEST;
            break;
        case RHI::ResourceAcessFlags::HOST_READ: return D3D12_BARRIER_ACCESS_COPY_SOURCE;
            break;
        case RHI::ResourceAcessFlags::HOST_WRITE: return D3D12_BARRIER_ACCESS_COPY_DEST;
            break;
        case RHI::ResourceAcessFlags::MEMORY_READ: return D3D12_BARRIER_ACCESS_COPY_SOURCE;
            break;
        case RHI::ResourceAcessFlags::MEMORY_WRITE: return D3D12_BARRIER_ACCESS_COPY_DEST;
            break;
        case RHI::ResourceAcessFlags::NONE: return D3D12_BARRIER_ACCESS_NO_ACCESS;
            break;
        default:
            break;
        }
    }
    D3D12_BARRIER_LAYOUT D3DBarrierLayout(ResourceLayout layout)
    {
        switch (layout)
        {
        case RHI::ResourceLayout::UNDEFINED: return D3D12_BARRIER_LAYOUT_UNDEFINED;
            break;
        case RHI::ResourceLayout::GENERAL: return D3D12_BARRIER_LAYOUT_COMMON;
            break;
        case RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL: return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
            break;
        case RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
            break;
        case RHI::ResourceLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL: return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
            break;
        case RHI::ResourceLayout::SHADER_READ_ONLY_OPTIMAL: return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
            break;
        case RHI::ResourceLayout::TRANSFER_SRC_OPTIMAL: return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
            break;
        case RHI::ResourceLayout::TRANSFER_DST_OPTIMAL: return D3D12_BARRIER_LAYOUT_COPY_DEST;
            break;
        case RHI::ResourceLayout::READ_ONLY_OPTIMAL: return D3D12_BARRIER_LAYOUT_GENERIC_READ;
            break;
        case RHI::ResourceLayout::PRESENT: return D3D12_BARRIER_LAYOUT_PRESENT;
            break;
        case RHI::ResourceLayout::VIDEO_DECODE_DST: return D3D12_BARRIER_LAYOUT_VIDEO_DECODE_WRITE;
            break;
        case RHI::ResourceLayout::VIDEO_DECODE_SRC: return D3D12_BARRIER_LAYOUT_VIDEO_DECODE_WRITE;
            break;
        case RHI::ResourceLayout::FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL: return D3D12_BARRIER_LAYOUT_SHADING_RATE_SOURCE;
            break;
        default:
            break;
        }
    }
    D3D12_BARRIER_SYNC D3DBarrierSync(PipelineStage stage)
    {
        switch (stage)
        {
        case RHI::PipelineStage::TOP_OF_PIPE_BIT: return D3D12_BARRIER_SYNC_NONE;
            break;
        case RHI::PipelineStage::DRAW_INDIRECT_BIT: return D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
            break;
        case RHI::PipelineStage::VERTEX_INPUT_BIT: return D3D12_BARRIER_SYNC_INDEX_INPUT;
            break;
        case RHI::PipelineStage::VERTEX_SHADER_BIT: return D3D12_BARRIER_SYNC_VERTEX_SHADING;
            break;
        case RHI::PipelineStage::TESSELLATION_CONTROL_SHADER_BIT: return D3D12_BARRIER_SYNC_NON_PIXEL_SHADING; // Replace with appropriate values
            break;
        case RHI::PipelineStage::TESSELLATION_EVALUATION_SHADER_BIT: return D3D12_BARRIER_SYNC_NON_PIXEL_SHADING; // Replace with appropriate values
            break;
        case RHI::PipelineStage::GEOMETRY_SHADER_BIT: return D3D12_BARRIER_SYNC_NON_PIXEL_SHADING; // Replace with appropriate values
            break;
        case RHI::PipelineStage::FRAGMENT_SHADER_BIT: return D3D12_BARRIER_SYNC_PIXEL_SHADING;
            break;
        case RHI::PipelineStage::COLOR_ATTACHMENT_OUTPUT_BIT: return D3D12_BARRIER_SYNC_RENDER_TARGET;
            break;
        case RHI::PipelineStage::COMPUTE_SHADER_BIT: return D3D12_BARRIER_SYNC_COMPUTE_SHADING;
            break;
        case RHI::PipelineStage::TRANSFER_BIT: return D3D12_BARRIER_SYNC_COPY;
            break;
        case RHI::PipelineStage::BOTTOM_OF_PIPE_BIT: return D3D12_BARRIER_SYNC_NONE;
            break;
        case RHI::PipelineStage::ALL_GRAPHICS_BIT: return D3D12_BARRIER_SYNC_ALL_SHADING;
            break;
        case RHI::PipelineStage::ALL_COMMANDS_BIT: return  D3D12_BARRIER_SYNC_ALL;
            break;
        case RHI::PipelineStage::NONE: return D3D12_BARRIER_SYNC_NONE;
            break;
        case RHI::PipelineStage::TRANSFORM_FEEDBACK_BIT_EXT:
            break;
        case RHI::PipelineStage::CONDITIONAL_RENDERING_BIT_EXT:
            break;
        case RHI::PipelineStage::ACCELERATION_STRUCTURE_BUILD_BIT_KHR:
            break;
        case RHI::PipelineStage::RAY_TRACING_SHADER_BIT_KHR:
            break;
        case RHI::PipelineStage::FRAGMENT_DENSITY_PROCESS_BIT_EXT:
            break;
        case RHI::PipelineStage::FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR:
            break;
        case RHI::PipelineStage::COMMAND_PREPROCESS_BIT_NV:
            break;
        case RHI::PipelineStage::TASK_SHADER_BIT_EXT:
            break;
        case RHI::PipelineStage::MESH_SHADER_BIT_EXT:
            break;
        case RHI::PipelineStage::FLAG_BITS_MAX_ENUM:
            break;
        default:
            break;
        }
    }
    RESULT GraphicsCommandList::Begin(CommandAllocator* allocator)
    {
        return ((ID3D12GraphicsCommandList*)ID)->Reset((ID3D12CommandAllocator*)allocator->ID, nullptr);
    }
    RESULT GraphicsCommandList::PipelineBarrier(PipelineStage syncBefore, PipelineStage syncAfter, std::uint32_t numBufferBarriers, BufferMemoryBarrier* pbufferBarriers, std::uint32_t numImageBarriers, TextureMemoryBarrier* pImageBarriers)
    {
        D3D12_BARRIER_GROUP group[2];
        UINT numBarriers = 0;
        D3D12_BUFFER_BARRIER Bbarr[5]{};
        D3D12_TEXTURE_BARRIER Tbarr[5]{};
        for (int i = 0; i < numBufferBarriers; i++)
        {
            //barr.AccessBefore = D3DAcessMode(pbufferBarriers[i].AccessFlagsBefore);
            //barr.AccessAfter = D3DAcessMode(desc->AccessFlagsAfter);
            //barr.LayoutBefore = D3DBarrierLayout(desc->oldLayout);
            //barr.LayoutAfter = D3DBarrierLayout(desc->newLayout);
            //auto mem = ConvertToDeviceFormat(&pbufferBarriers[i]);
            //barr = *(D3D12_BUFFER_BARRIER*)&mem;
            Bbarr[i].SyncBefore = D3DBarrierSync(syncBefore);
            Bbarr[i].SyncAfter = D3DBarrierSync(syncAfter);
        }
        if (numBufferBarriers)
        {
            group[0].NumBarriers = numBufferBarriers;
            group[0].Type = D3D12_BARRIER_TYPE_BUFFER;
            group[0].pBufferBarriers = Bbarr;
            numBarriers++;
        }
        for (int i = 0; i < numImageBarriers; i++)
        {
            Tbarr[i] = ConvertToDeviceFormat(&pImageBarriers[i]);
            Tbarr[i].SyncBefore = D3DBarrierSync(syncBefore);
            Tbarr[i].SyncAfter = D3DBarrierSync(syncAfter);
        }
        if (numImageBarriers)
        {
            group[numBarriers].NumBarriers = numImageBarriers;
            group[numBarriers].Type = D3D12_BARRIER_TYPE_TEXTURE;
            group[numBarriers].pTextureBarriers = Tbarr;
            numBarriers++;
        }
        ((ID3D12GraphicsCommandList7*)ID)->Barrier(numBarriers, group);
        return RESULT();
    }
    RESULT GraphicsCommandList::BeginRendering(const RenderingBeginDesc* desc)
    {
        RECT rect;
        rect.left = desc->renderingArea.offset.x;
        rect.right = rect.left + desc->renderingArea.size.x;
        rect.top = desc->renderingArea.offset.y;
        rect.bottom = rect.top + desc->renderingArea.size.y;
        ((ID3D12GraphicsCommandList*)ID)->RSSetScissorRects(1, &rect);
        D3D12_CPU_DESCRIPTOR_HANDLE handle[5];
        for (int i = 0; i < desc->numColorAttachments; i++)
        {
            if (desc->pColorAttachments[i].loadOp == LoadOp::Clear)
            {
                ((ID3D12GraphicsCommandList*)ID)->ClearRenderTargetView({ desc->pColorAttachments[i].ImageView.val }, (float*)&desc->pColorAttachments[i].clearColor, 1, &rect);
            }
            handle[i].ptr = desc->pColorAttachments[i].ImageView.val;
        }
        if (desc->pDepthStencilAttachment)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = { desc->pDepthStencilAttachment->ImageView.val };
            ((ID3D12GraphicsCommandList*)ID)->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 1, & rect);
            ((ID3D12GraphicsCommandList*)ID)->OMSetRenderTargets(desc->numColorAttachments, handle, 0, &dsvHandle);
        }
        else
        {
            ((ID3D12GraphicsCommandList*)ID)->OMSetRenderTargets(desc->numColorAttachments, handle, 0, 0);
        }
        return RESULT();
    }
    RESULT GraphicsCommandList::EndRendering()
    {
        return RESULT();
    }
    RESULT GraphicsCommandList::End()
    {
        return ((ID3D12GraphicsCommandList*)ID)->Close();
    }
    RESULT GraphicsCommandList::SetPipelineState(PipelineStateObject* pso)
    {
        ((ID3D12GraphicsCommandList*)ID)->SetPipelineState((ID3D12PipelineState*)pso->ID);
        ((ID3D12GraphicsCommandList*)ID)->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        strides = ((D3D12PipelineStateObject*)pso)->strides.data();
        return 0;
    }
    RESULT GraphicsCommandList::SetScissorRects(uint32_t numRects, Area2D* rects)
    {
        RECT rect[4];
        for (int i = 0; i < numRects; i++)
        {
            rect[i].left = rects[i].offset.x;
            rect[i].right = rect[i].left + rects[i].size.x;
            rect[i].top = rects[i].offset.y;
            rect[i].bottom = rect[i].top + rects[i].size.y;
        }
        ((ID3D12GraphicsCommandList*)ID)->RSSetScissorRects(numRects, rect);
        return 0;
    }
    RESULT GraphicsCommandList::SetViewports(uint32_t numViewports, Viewport* viewports)
    {
        D3D12_VIEWPORT vp[4];
        for (int i = 0; i < numViewports; i++)
        {
            vp[i].TopLeftX = viewports[i].x;
            vp[i].TopLeftY = viewports[i].y;
            vp[i].Height = viewports[i].height;
            vp[i].Width = viewports[i].width;
            vp[i].MinDepth = viewports[i].minDepth;
            vp[i].MaxDepth = viewports[i].maxDepth;
        }
        ((ID3D12GraphicsCommandList*)ID)->RSSetViewports(numViewports, vp);
        return RESULT();
    }
    RESULT GraphicsCommandList::Draw(uint32_t numVertices, uint32_t numInstances, uint32_t firstVertex, uint32_t firstInstance)
    {
        ((ID3D12GraphicsCommandList*)ID)->DrawInstanced(numVertices, numInstances, firstVertex, firstInstance);
        return RESULT();
    }
    RESULT GraphicsCommandList::BindVertexBuffers(uint32_t startSlot, uint32_t numBuffers, const Internal_ID* buffers)
    {
        D3D12_VERTEX_BUFFER_VIEW views[5];
        for (int i = 0; i < numBuffers; i++)
        {
            auto desc = ((ID3D12Resource*)buffers[i])->GetDesc();
            views[0].BufferLocation = ((ID3D12Resource*)buffers[i])->GetGPUVirtualAddress();
            views[0].SizeInBytes = desc.Width;
            views[0].StrideInBytes = strides[startSlot + i];
        }
        ((ID3D12GraphicsCommandList*)ID)->IASetVertexBuffers(startSlot, numBuffers, views);
        return RESULT();
    }
    RESULT GraphicsCommandList::DrawIndexed(uint32_t IndexCount, uint32_t InstanceCount, uint32_t startIndexLocation, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        ((ID3D12GraphicsCommandList*)ID)->DrawIndexedInstanced(IndexCount, InstanceCount, startIndexLocation, startVertexLocation, startInstanceLocation);
        return RESULT();
    }
    RESULT RHI::GraphicsCommandList::BindIndexBuffer(const Buffer* buffer, uint32_t offset)
    {
        D3D12_INDEX_BUFFER_VIEW view;
        auto desc = ((ID3D12Resource*)buffer->ID)->GetDesc();
        view.BufferLocation = ((ID3D12Resource*)buffer->ID)->GetGPUVirtualAddress() + offset;
        view.Format = DXGI_FORMAT_R32_UINT; //todo
        view.SizeInBytes = desc.Width;
        ((ID3D12GraphicsCommandList*)ID)->IASetIndexBuffer(&view);
        return RESULT();
    }
    RESULT GraphicsCommandList::SetRootSignature(RootSignature* rs)
    {
        ((ID3D12GraphicsCommandList*)ID)->SetGraphicsRootSignature((ID3D12RootSignature*)rs->ID);
        return RESULT();
    }
    RESULT GraphicsCommandList::BindDescriptorSet(RootSignature* rs, DescriptorSet* set, std::uint32_t setIndex)
    {
        //TODO
        D3D12_GPU_DESCRIPTOR_HANDLE handle;
        handle.ptr = ((D3D12DescriptorSet*)set)->gpu_handle;
        ((ID3D12GraphicsCommandList*)ID)->SetGraphicsRootDescriptorTable(rootParamIndex, handle);
        return RESULT();
    }
    RESULT GraphicsCommandList::SetDescriptorHeap(DescriptorHeap* heap)
    {
        ID3D12DescriptorHeap* heaps[] = { (ID3D12DescriptorHeap*)heap->ID };
        ((ID3D12GraphicsCommandList*)ID)->SetDescriptorHeaps(1, heaps);
        return 0;
    }
    RESULT GraphicsCommandList::CopyBufferRegion(uint32_t srcOffset, uint32_t dstOffset, uint32_t size, Buffer* srcBuffer, Buffer* dstBuffer)
    {
        ((ID3D12GraphicsCommandList*)ID)->CopyBufferRegion((ID3D12Resource*)dstBuffer->ID, dstOffset, (ID3D12Resource*)srcBuffer->ID, srcOffset, size);
        return 0;
    }
    RESULT GraphicsCommandList::CopyBufferToImage(uint32_t srcOffset, SubResourceRange dstRange, Offset3D imgOffset, Extent3D imgSize, Buffer* buffer, Texture* texture)
    {
        D3D12_TEXTURE_COPY_LOCATION pSrcCopy;
        ID3D12Device* device;
        ((ID3D12Resource*)texture->ID)->GetDevice(IID_PPV_ARGS(&device));
        auto desc = ((ID3D12Resource*)buffer->ID)->GetDesc();
        UINT numRows;
        UINT64 rowSize;
        UINT64 totalSize;
        device->GetCopyableFootprints(&desc, 0, 1, srcOffset, &pSrcCopy.PlacedFootprint,&numRows, &rowSize, &totalSize);
        pSrcCopy.pResource = (ID3D12Resource*)buffer->ID;
        pSrcCopy.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        desc = ((ID3D12Resource*)texture->ID)->GetDesc();
        D3D12_TEXTURE_COPY_LOCATION pDstCopy;
        pDstCopy.pResource = (ID3D12Resource*)texture->ID;
        pDstCopy.SubresourceIndex = D3D12CalcSubresource(dstRange.IndexOrFirstMipLevel, dstRange.FirstArraySlice, 0, desc.MipLevels, desc.DepthOrArraySize);//todo
        pDstCopy.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        pSrcCopy.PlacedFootprint.Footprint.Format = desc.Format;
        D3D12_BOX size;
        size.left = 0;
        size.right = imgSize.width;
        size.top = 0;
        size.bottom = imgSize.height;
        size.front = 0;
        size.back = imgSize.depth;
        pSrcCopy.PlacedFootprint.Footprint.Width = imgSize.width;
        pSrcCopy.PlacedFootprint.Footprint.Height = imgSize.height;
        pSrcCopy.PlacedFootprint.Footprint.Depth = imgSize.depth;
        pSrcCopy.PlacedFootprint.Footprint.RowPitch = 4 * imgSize.width;
        ((ID3D12GraphicsCommandList*)ID)->CopyTextureRegion(&pDstCopy, imgOffset.width,imgOffset.height, imgOffset.depth, &pSrcCopy, &size);
        return 0;
    }
    RESULT GraphicsCommandList::BindDynamicDescriptor(RootSignature* rs, const DynamicDescriptor* set, std::uint32_t rootParamIndex, std::uint32_t offset)
    {
        ((ID3D12GraphicsCommandList*)ID)->SetGraphicsRootConstantBufferView(rootParamIndex, ((D3D12DynamicDescriptor*)set)->address + offset);
        return RESULT();
    }
    
}
