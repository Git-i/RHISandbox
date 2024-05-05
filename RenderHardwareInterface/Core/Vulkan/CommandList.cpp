#include "pch.h"
#include "../CommandList.h"
#include "volk.h"
#include "VulkanSpecific.h"
namespace RHI
{
    static VkAttachmentLoadOp VulkanLoadOp(LoadOp op)
    {
        switch (op)
        {
        case(LoadOp::Load): return VK_ATTACHMENT_LOAD_OP_LOAD;
        case(LoadOp::Clear): return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case(LoadOp::DontCare): return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default: return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }
    static VkAttachmentStoreOp VulkanStoreOp(StoreOp op)
    {
        switch (op)
        {
        case(StoreOp::Store): return VK_ATTACHMENT_STORE_OP_STORE;
        case(StoreOp::DontCare): return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default: return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
    }
    RESULT GraphicsCommandList::Begin(CommandAllocator* allocator)
    {
        VkCommandBufferBeginInfo bufferBeginInfo = {};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        VkCommandBufferAllocateInfo Info = {};
        Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        Info.commandPool = (VkCommandPool)allocator->ID;
        Info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        Info.commandBufferCount = 1;
        //vkFreeCommandBuffers((VkDevice)Device_ID, (VkCommandPool)(((vGraphicsCommandList*)this)->m_allocator), 1, (VkCommandBuffer*)&ID);
        //vkResetCommandBuffer((VkCommandBuffer)ID, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        vkAllocateCommandBuffers((VkDevice)((vDevice*)device)->ID, &Info, (VkCommandBuffer*)&ID);
        ((vCommandAllocator*)allocator)->m_pools.push_back(ID);
        return vkBeginCommandBuffer((VkCommandBuffer)ID, &bufferBeginInfo);
    }
    uint32_t QueueFamilyInd(vDevice* device, QueueFamily fam)
    {
        switch (fam)
        {
        case RHI::QueueFamily::Graphics: return device->indices.graphicsIndex;
            break;
        case RHI::QueueFamily::Compute: return device->indices.computeIndex;
            break;
        case RHI::QueueFamily::Copy: return device->indices.copyIndex;
            break;
        case RHI::QueueFamily::Ignored: return VK_QUEUE_FAMILY_IGNORED;
            break;
        default: return VK_QUEUE_FAMILY_IGNORED;
            break;
        }
    }
    RESULT GraphicsCommandList::PipelineBarrier(PipelineStage syncBefore, PipelineStage syncAfter, std::uint32_t numBufferBarriers, BufferMemoryBarrier* bufferBarrier,std::uint32_t numImageBarriers, TextureMemoryBarrier* pImageBarriers)
    {
        VkBufferMemoryBarrier BufferBarr[10]{};
        VkImageMemoryBarrier ImageBarr[100]{};
        for (uint32_t i = 0; i < numBufferBarriers; i++)
        {
            BufferBarr[i].buffer = (VkBuffer)bufferBarrier[i].buffer->ID;
            BufferBarr[i].srcAccessMask = (VkAccessFlags)bufferBarrier[i].AccessFlagsBefore;
            BufferBarr[i].dstAccessMask = (VkAccessFlags)bufferBarrier[i].AccessFlagsAfter;
            BufferBarr[i].srcQueueFamilyIndex = QueueFamilyInd((vDevice*)device, bufferBarrier[i].previousQueue);
            BufferBarr[i].dstQueueFamilyIndex = QueueFamilyInd((vDevice*)device, bufferBarrier[i].nextQueue);
            if (BufferBarr[i].srcQueueFamilyIndex == BufferBarr[i].dstQueueFamilyIndex)
                BufferBarr[i].srcQueueFamilyIndex = BufferBarr[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            BufferBarr[i].offset = bufferBarrier[i].offset;
            BufferBarr[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            BufferBarr[i].size = bufferBarrier[i].size;
        }
        for (uint32_t i = 0; i < numImageBarriers; i++)
        {
            ImageBarr[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            ImageBarr[i].image = (VkImage)pImageBarriers[i].texture->ID;
            ImageBarr[i].newLayout = (VkImageLayout)pImageBarriers[i].newLayout;
            ImageBarr[i].oldLayout = (VkImageLayout)pImageBarriers[i].oldLayout;
            ImageBarr[i].srcAccessMask = (VkAccessFlags)pImageBarriers[i].AccessFlagsBefore;
            ImageBarr[i].dstAccessMask = (VkAccessFlags)pImageBarriers[i].AccessFlagsAfter;
            ImageBarr[i].srcQueueFamilyIndex = QueueFamilyInd((vDevice*)device, pImageBarriers[i].previousQueue);
            ImageBarr[i].dstQueueFamilyIndex = QueueFamilyInd((vDevice*)device, pImageBarriers[i].nextQueue);
            VkImageSubresourceRange range;
            range.aspectMask = (VkImageAspectFlags)pImageBarriers[i].subresourceRange.imageAspect;
            range.baseMipLevel = pImageBarriers[i].subresourceRange.IndexOrFirstMipLevel;
            range.baseArrayLayer = pImageBarriers[i].subresourceRange.FirstArraySlice;
            range.layerCount = pImageBarriers[i].subresourceRange.NumArraySlices;
            range.levelCount = pImageBarriers[i].subresourceRange.NumMipLevels;
            ImageBarr[i].subresourceRange = range;
            if (ImageBarr[i].srcQueueFamilyIndex == ImageBarr[i].dstQueueFamilyIndex)
                ImageBarr[i].srcQueueFamilyIndex = ImageBarr[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        vkCmdPipelineBarrier((VkCommandBuffer)ID, (VkFlags)syncBefore, (VkFlags)syncAfter, 0, 0, nullptr, numBufferBarriers, BufferBarr, numImageBarriers, ImageBarr);
        return RESULT();
    }
    VkAttachmentLoadOp vloadOp(LoadOp op)
    {
        switch (op)
        {
        case RHI::LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case RHI::LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        case RHI::LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        default:
            break;
        }
    }
    VkAttachmentStoreOp vStoreOp(StoreOp op)
    {
        switch (op)
        {
        case RHI::StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
            break;
        case RHI::StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            break;
        default:
            break;
        }
    }
    RESULT GraphicsCommandList::BeginRendering(const RenderingBeginDesc* desc)
    {
        VkRenderingAttachmentInfo Attachmentinfos[5] = {};
        for (int i = 0; i < desc->numColorAttachments; i++)
        {
            Attachmentinfos[i].clearValue.color = *(VkClearColorValue*)&desc->pColorAttachments[i].clearColor;
            Attachmentinfos[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            Attachmentinfos[i].imageView = *(VkImageView*)desc->pColorAttachments[i].ImageView.ptr;
            Attachmentinfos[i].loadOp = vloadOp(desc->pColorAttachments[i].loadOp);
            Attachmentinfos[i].storeOp = vStoreOp(desc->pColorAttachments[i].storeOp);
            Attachmentinfos[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        }
        VkRect2D render_area;
        render_area.offset = { desc->renderingArea.offset.x,desc->renderingArea.offset.y };
        render_area.extent = { desc->renderingArea.size.x,desc->renderingArea.size.y };
        VkRenderingInfo info = {};
        info.pColorAttachments = Attachmentinfos;
        VkRenderingAttachmentInfo depthAttach{};
        if (desc->pDepthStencilAttachment)
        {
            depthAttach.clearValue.color = *(VkClearColorValue*)&desc->pDepthStencilAttachment->clearColor;
            depthAttach.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttach.imageView = *(VkImageView*)desc->pDepthStencilAttachment->ImageView.ptr;
            depthAttach.loadOp = vloadOp(desc->pDepthStencilAttachment->loadOp);
            depthAttach.storeOp = vStoreOp(desc->pDepthStencilAttachment->storeOp);
            depthAttach.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            info.pDepthAttachment = &depthAttach;
        }
        info.colorAttachmentCount = desc->numColorAttachments;
        info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        info.renderArea = render_area;
        info.layerCount = 1;
        vkCmdBeginRenderingKHR((VkCommandBuffer)ID, &info);
        return RESULT();
    }
    RESULT GraphicsCommandList::EndRendering()
    {
        vkCmdEndRenderingKHR((VkCommandBuffer)ID);
        return RESULT();
    }
    RESULT GraphicsCommandList::End()
    {
        return vkEndCommandBuffer((VkCommandBuffer)ID);
    }
    RESULT GraphicsCommandList::SetPipelineState(PipelineStateObject* pso)
    {
        vkCmdBindPipeline((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)pso->ID);
        return 0;
    }
    RESULT GraphicsCommandList::SetScissorRects(uint32_t numRects, Area2D* rects)
    {
        VkRect2D scr[4];
        for (int i = 0; i < numRects; i++)
        {
            scr[i].extent = { rects[i].size.x, rects[i].size.y};
            scr[i].offset = { rects[i].offset.x,rects[i].offset.y};
        }
        vkCmdSetScissor((VkCommandBuffer)ID, 0, numRects, scr);
        return 0;
    }
    RESULT GraphicsCommandList::SetViewports(uint32_t numViewports, Viewport* viewports)
    {
        VkViewport vp[4];
        for (int i = 0; i < numViewports; i++)
        {
            vp[i].x = viewports[i].x;
            vp[i].height = -viewports[i].height + viewports[i].y;
            vp[i].y = viewports[i].height + viewports[i].y;
            vp[i].width = viewports[i].width;
            vp[i].minDepth = viewports[i].minDepth;
            vp[i].maxDepth = viewports[i].maxDepth;
        }
        vkCmdSetViewport((VkCommandBuffer)ID, 0, numViewports, vp);
        return RESULT();
    }
    RESULT GraphicsCommandList::Draw(uint32_t numVertices, uint32_t numInstances, uint32_t firstVertex, uint32_t firstIndex)
    {
        vkCmdDraw((VkCommandBuffer)ID, numVertices, numInstances, firstVertex, firstIndex);
        return RESULT();
    }
    RESULT GraphicsCommandList::BindVertexBuffers(uint32_t startSlot, uint32_t numBuffers, const Internal_ID* buffers)
    {
        VkDeviceSize l = 0;
        vkCmdBindVertexBuffers((VkCommandBuffer)ID, startSlot, numBuffers, (VkBuffer*)buffers, &l);
        return RESULT();
    }
    RESULT GraphicsCommandList::SetRootSignature(RootSignature* rs)
    {
        return RESULT();
        
    }
    RESULT GraphicsCommandList::BindDynamicDescriptor(RootSignature* rs, const DynamicDescriptor* set, std::uint32_t setIndex, std::uint32_t offset)
    {
        VkDescriptorSet sets;
        sets = (VkDescriptorSet)set->ID;
        vkCmdBindDescriptorSets((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)rs->ID, setIndex, 1, &sets,1,&offset);
        return 0;
    }
    RESULT GraphicsCommandList::BindDescriptorSet(RootSignature* rs, DescriptorSet* set, std::uint32_t setIndex)
    {
        VkDescriptorSet sets;
        sets = (VkDescriptorSet)set->ID;
        vkCmdBindDescriptorSets((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)rs->ID, setIndex, 1, &sets, 0, 0);
        return RESULT();
    }
    RESULT GraphicsCommandList::SetDescriptorHeap(DescriptorHeap* heap)
    {
        return 0;
    }
    RESULT GraphicsCommandList::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
    {
        vkCmdDispatch((VkCommandBuffer)ID, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
        return RESULT();
    }
    RESULT GraphicsCommandList::BindIndexBuffer(const Buffer* buffer, uint32_t offset)
    {
        vkCmdBindIndexBuffer((VkCommandBuffer)ID, (VkBuffer)buffer->ID, offset, VK_INDEX_TYPE_UINT32);
        return RESULT();
    }
    RESULT GraphicsCommandList::BlitTexture(Texture* src, Texture* dst, Extent3D srcSize, Offset3D srcOffset, Extent3D dstSize, Offset3D dstOffset)
    {
        VkImageSubresourceLayers lyrs;
        lyrs.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        lyrs.baseArrayLayer = 0;
        lyrs.layerCount = 1;
        lyrs.mipLevel = 0;
        VkImageBlit blt;
        blt.dstOffsets[0] = { dstOffset.width, dstOffset.height, dstOffset.depth };
        blt.dstOffsets[1] = { (int)(dstOffset.width + dstSize.width), (int)(dstOffset.height + dstSize.height),(int)(dstOffset.depth + dstSize.depth) };
        blt.srcOffsets[0] = { srcOffset.width, srcOffset.height, srcOffset.depth};
        blt.srcOffsets[1] = 
        { (int)(srcOffset.width + srcSize.width), (int)(srcOffset.height + srcSize.height),(int)(srcOffset.depth + srcSize.depth)};
        blt.srcSubresource = blt.dstSubresource = lyrs;
        vkCmdBlitImage((VkCommandBuffer)ID, (VkImage)src->ID, VK_IMAGE_LAYOUT_GENERAL, (VkImage)dst->ID, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blt, VK_FILTER_LINEAR);
        return 0;
    }
    RESULT GraphicsCommandList::MarkBuffer(Buffer* buffer, uint32_t offset, uint32_t val)
    {
        vkCmdFillBuffer((VkCommandBuffer)ID, (VkBuffer)buffer->ID, offset, sizeof(uint32_t), val);
        return RESULT();
    }
    RESULT GraphicsCommandList::MarkBuffer(DebugBuffer* buffer, uint32_t val)
    {
        VkAfterCrash_CmdWriteMarker((VkCommandBuffer)ID, (VkAfterCrash_Buffer)buffer->ID, 0, val);
        return 0;
    }
    RESULT GraphicsCommandList::DrawIndexed(uint32_t IndexCount, uint32_t InstanceCount, uint32_t startIndexLocation, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        vkCmdDrawIndexed((VkCommandBuffer)ID, IndexCount, InstanceCount, startIndexLocation, startVertexLocation, startInstanceLocation);
        return 0;
    }
    RESULT GraphicsCommandList::CopyBufferRegion(uint32_t srcOffset, uint32_t dstOffset, uint32_t size, Buffer* srcBuffer, Buffer* dstBuffer)
    {
        VkBufferCopy copy{};
        copy.size = size;
        copy.srcOffset = srcOffset;
        copy.dstOffset = dstOffset;
        vkCmdCopyBuffer((VkCommandBuffer)ID, (VkBuffer)srcBuffer->ID, (VkBuffer)dstBuffer->ID, 1, &copy);
        return RESULT();
    }
    RESULT GraphicsCommandList::CopyBufferToImage(uint32_t srcOffset, SubResourceRange dstRange, Offset3D imgOffset, Extent3D imgSize, Buffer* buffer, Texture* texture)
    {
        VkBufferImageCopy copy{};
        copy.bufferImageHeight = 0;
        copy.bufferOffset = srcOffset;
        copy.bufferRowLength = 0;
        copy.imageExtent = { imgSize.width, imgSize.height, imgSize.depth };
        copy.imageOffset = { imgOffset.width, imgOffset.height, imgOffset.depth };
        copy.imageSubresource.aspectMask = (VkImageAspectFlags)dstRange.imageAspect;
        copy.imageSubresource.baseArrayLayer = dstRange.FirstArraySlice;
        copy.imageSubresource.layerCount = dstRange.NumArraySlices;
        copy.imageSubresource.mipLevel = dstRange.IndexOrFirstMipLevel;
        vkCmdCopyBufferToImage((VkCommandBuffer)ID, (VkBuffer)buffer->ID, (VkImage)texture->ID, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        return RESULT();
    }

    RESULT GraphicsCommandList::CopyTextureRegion(SubResourceRange srcRange, SubResourceRange dstRange, Offset3D srcOffset, Offset3D dstOffset, Extent3D extent, Texture* src, Texture* dst)
    {
        VkImageCopy copy{};
        copy.dstOffset = { dstOffset.width, dstOffset.height,dstOffset.depth };
        copy.srcOffset = { srcOffset.width, srcOffset.height, dstOffset.depth };
        copy.extent = { extent.width,extent.height,extent.depth };
        copy.srcSubresource.aspectMask = (VkImageAspectFlags)srcRange.imageAspect;
        copy.srcSubresource.baseArrayLayer = srcRange.FirstArraySlice;
        copy.srcSubresource.layerCount = srcRange.NumArraySlices;
        copy.srcSubresource.mipLevel = srcRange.IndexOrFirstMipLevel;
        copy.dstSubresource.aspectMask = (VkImageAspectFlags)dstRange.imageAspect;
        copy.dstSubresource.baseArrayLayer = dstRange.FirstArraySlice;
        copy.dstSubresource.layerCount = dstRange.NumArraySlices;
        copy.dstSubresource.mipLevel = dstRange.IndexOrFirstMipLevel;
        vkCmdCopyImage((VkCommandBuffer)ID, (VkImage)src->ID, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, (VkImage)dst->ID, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        return RESULT();
        
    }

    RESULT GraphicsCommandList::ReleaseBarrier(PipelineStage syncBefore, PipelineStage syncAfter, std::uint32_t numBufferBarriers, BufferMemoryBarrier* pbufferBarriers, std::uint32_t numImageBarriers, TextureMemoryBarrier* pImageBarriers)
    {
        VkBufferMemoryBarrier BufferBarr[10]{};
        VkImageMemoryBarrier ImageBarr[100]{};
        int bn = 0, tn = 0;
        for (uint32_t i = 0; i < numBufferBarriers; i++)
        {
            if (QueueFamilyInd((vDevice*)device, pbufferBarriers[i].previousQueue) == QueueFamilyInd((vDevice*)device, pbufferBarriers[i].nextQueue))
                continue;
            BufferBarr[bn].buffer = (VkBuffer)pbufferBarriers[i].buffer->ID;
            BufferBarr[bn].srcAccessMask = (VkAccessFlags)pbufferBarriers[i].AccessFlagsBefore;
            BufferBarr[bn].dstAccessMask = (VkAccessFlags)pbufferBarriers[i].AccessFlagsAfter;
            BufferBarr[bn].srcQueueFamilyIndex = QueueFamilyInd((vDevice*)device, pbufferBarriers[i].previousQueue);
            BufferBarr[bn].dstQueueFamilyIndex = QueueFamilyInd((vDevice*)device, pbufferBarriers[i].nextQueue);
            BufferBarr[bn].offset = pbufferBarriers[i].offset;
            BufferBarr[bn].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            BufferBarr[bn].size = pbufferBarriers[i].size;
            bn++;
        }
        for (uint32_t i = 0; i < numImageBarriers; i++)
        {
            if(QueueFamilyInd((vDevice*)device, pImageBarriers[i].previousQueue) == QueueFamilyInd((vDevice*)device, pImageBarriers[i].nextQueue))
                continue;
            ImageBarr[tn].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            ImageBarr[tn].image = (VkImage)pImageBarriers[i].texture->ID;
            ImageBarr[tn].newLayout = (VkImageLayout)pImageBarriers[i].newLayout;
            ImageBarr[tn].oldLayout = (VkImageLayout)pImageBarriers[i].oldLayout;
            ImageBarr[tn].srcAccessMask = (VkAccessFlags)pImageBarriers[i].AccessFlagsBefore;
            ImageBarr[tn].dstAccessMask = (VkAccessFlags)pImageBarriers[i].AccessFlagsAfter;
            ImageBarr[tn].srcQueueFamilyIndex = QueueFamilyInd((vDevice*)device, pImageBarriers[i].previousQueue);
            ImageBarr[tn].dstQueueFamilyIndex = QueueFamilyInd((vDevice*)device, pImageBarriers[i].nextQueue);
            VkImageSubresourceRange range;
            range.aspectMask = (VkImageAspectFlags)pImageBarriers[i].subresourceRange.imageAspect;
            range.baseMipLevel = pImageBarriers[i].subresourceRange.IndexOrFirstMipLevel;
            range.baseArrayLayer = pImageBarriers[i].subresourceRange.FirstArraySlice;
            range.layerCount = pImageBarriers[i].subresourceRange.NumArraySlices;
            range.levelCount = pImageBarriers[i].subresourceRange.NumMipLevels;
            ImageBarr[tn].subresourceRange = range;
            tn++;
        }
        if(bn+tn)
            vkCmdPipelineBarrier((VkCommandBuffer)ID, (VkFlags)syncBefore, (VkFlags)syncAfter, 0, 0, nullptr, numBufferBarriers, BufferBarr, numImageBarriers, ImageBarr);
        return RESULT();
    }

    RESULT GraphicsCommandList::SetComputePipeline(ComputePipeline* cp)
    {
        vkCmdBindPipeline((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_COMPUTE, (VkPipeline)cp->ID);
        return RESULT();
    }
    RESULT GraphicsCommandList::BindComputeDescriptorSet(RootSignature* rs, DescriptorSet* set, std::uint32_t setIndex)
    {
        VkDescriptorSet sets;
        sets = (VkDescriptorSet)set->ID;
        vkCmdBindDescriptorSets((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_COMPUTE, (VkPipelineLayout)rs->ID, setIndex, 1, &sets, 0, 0);
        return RESULT();
    }
    RESULT GraphicsCommandList::BindComputeDynamicDescriptor(RootSignature* rs, const DynamicDescriptor* set, std::uint32_t setIndex, std::uint32_t offset)
    {
        VkDescriptorSet sets;
        sets = (VkDescriptorSet)set->ID;
        vkCmdBindDescriptorSets((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_COMPUTE, (VkPipelineLayout)rs->ID, setIndex, 1, &sets, 1, &offset);
        return 0;
    }
}
