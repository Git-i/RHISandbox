#include "pch.h"
#include "../CommandList.h"
#include "volk.h"
#include <atlalloc.h>
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
    void CommandList::Destroy()
    {
    }
    RESULT GraphicsCommandList::Begin(CommandAllocator allocator)
    {
        VkCommandBufferBeginInfo bufferBeginInfo = {};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        VkCommandBufferAllocateInfo Info = {};
        Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        Info.commandPool = (VkCommandPool)allocator.ID;
        Info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        Info.commandBufferCount = 1;
        vkFreeCommandBuffers((VkDevice)Device_ID, (VkCommandPool)m_allocator, 1, (VkCommandBuffer*)&ID);
        vkAllocateCommandBuffers((VkDevice)Device_ID, &Info, (VkCommandBuffer*)&ID);
        m_allocator = allocator.ID;
        return vkBeginCommandBuffer((VkCommandBuffer)ID, &bufferBeginInfo);
    }
    RESULT GraphicsCommandList::PipelineBarrier(PipelineStage syncBefore, PipelineStage syncAfter, std::uint32_t numBufferBarriers, BufferMemoryBarrier* bufferBarrier,std::uint32_t numImageBarriers, TextureMemoryBarrier* pImageBarriers)
    {
        VkBufferMemoryBarrier BufferBarr[5]{};
        VkImageMemoryBarrier ImageBarr[5]{};
        for (uint32_t i = 0; i < numBufferBarriers; i++)
        {
        }
        for (uint32_t i = 0; i < numImageBarriers; i++)
        {
            ImageBarr[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            ImageBarr[i].image = (VkImage)pImageBarriers[i].texture.ID;
            ImageBarr[i].newLayout = (VkImageLayout)pImageBarriers[i].newLayout;
            ImageBarr[i].oldLayout = (VkImageLayout)pImageBarriers[i].oldLayout;
            ImageBarr[i].srcAccessMask = (VkAccessFlags)pImageBarriers[i].AccessFlagsBefore;
            ImageBarr[i].dstAccessMask = (VkAccessFlags)pImageBarriers[i].AccessFlagsAfter;
            ImageBarr[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            ImageBarr[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            VkImageSubresourceRange range;
            range.aspectMask = (VkImageAspectFlags)pImageBarriers[i].subresourceRange.imageAspect;
            range.baseMipLevel = pImageBarriers[i].subresourceRange.IndexOrFirstMipLevel;
            range.baseArrayLayer = pImageBarriers[i].subresourceRange.FirstArraySlice;
            range.layerCount = pImageBarriers[i].subresourceRange.NumArraySlices;
            range.levelCount = pImageBarriers[i].subresourceRange.NumMipLevels;
            ImageBarr[i].subresourceRange = range;
        }
        vkCmdPipelineBarrier((VkCommandBuffer)ID, (VkFlags)syncBefore, (VkFlags)syncAfter, 0, 0, nullptr, numBufferBarriers, (VkBufferMemoryBarrier*)BufferBarr, numImageBarriers, (VkImageMemoryBarrier*)ImageBarr);
        return RESULT();
    }
    RESULT GraphicsCommandList::BeginRendering(const RenderingBeginDesc* desc)
    {
        for (int i = 0; i < desc->numColorAttachments; i++)
        {
            desc->pColorAttachmentsMem[i] = ConvertToDeviceFormat(&desc->pColorAttachments[i]);
        }
        VkRect2D render_area;
        render_area.offset = { desc->renderingArea.offset.x,desc->renderingArea.offset.y };
        render_area.extent = { desc->renderingArea.size.x,desc->renderingArea.size.y };
        VkRenderingInfo info = {};
        info.pColorAttachments = (VkRenderingAttachmentInfo*)desc->pColorAttachmentsMem;
        if (desc->pDepthStencilAttachment)
        {
            RenderingAttachmentDescStorage depthAttach = ConvertToDeviceFormat(desc->pDepthStencilAttachment);
            info.pDepthAttachment = (VkRenderingAttachmentInfo*)&depthAttach;
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
    RenderingAttachmentDescStorage RHI_API ConvertToDeviceFormat(const RenderingAttachmentDesc* desc)
    {
        VkRenderingAttachmentInfoKHR color_attachment_info = {};
        color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        color_attachment_info.imageView = (*(VkImageView*)desc->ImageView.ptr);
        color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        color_attachment_info.loadOp = VulkanLoadOp(desc->loadOp);
        color_attachment_info.storeOp = VulkanStoreOp(desc->storeOp);
        color_attachment_info.clearValue = {desc->clearColor.r, desc->clearColor.g, desc->clearColor.b, desc->clearColor.a};
        return *((RenderingAttachmentDescStorage*)(&color_attachment_info));
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
    RESULT GraphicsCommandList::BindVertexBuffers(uint32_t startSlot, uint32_t numBuffers, Internal_ID* buffers)
    {
        VkDeviceSize l = 0;
        vkCmdBindVertexBuffers((VkCommandBuffer)ID, startSlot, numBuffers, (VkBuffer*)buffers, &l);
        return RESULT();
    }
    RESULT GraphicsCommandList::SetRootSignature(RootSignature* rs)
    {
        return RESULT();
    }
    RESULT GraphicsCommandList::BindDescriptorSet(RootSignature rs, DescriptorSet set, std::uint32_t rootParamIndex)
    {
        VkDescriptorSet sets;
        sets = (VkDescriptorSet)set.ID;
        vkCmdBindDescriptorSets((VkCommandBuffer)ID, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)rs.ID, rootParamIndex, 1, &sets, 0, 0);
        return RESULT();
    }
    RESULT GraphicsCommandList::SetDescriptorHeap(DescriptorHeap heap)
    {
        return 0;
    }
    RESULT GraphicsCommandList::BindIndexBuffer(Buffer buffer, uint32_t offset)
    {
        vkCmdBindIndexBuffer((VkCommandBuffer)ID, (VkBuffer)buffer.ID, offset, VK_INDEX_TYPE_UINT16);
        return RESULT();
    }
    RESULT GraphicsCommandList::DrawIndexed(uint32_t IndexCount, uint32_t InstanceCount, uint32_t startIndexLocation, uint32_t startVertexLocation, uint32_t startInstanceLocation)
    {
        vkCmdDrawIndexed((VkCommandBuffer)ID, IndexCount, InstanceCount, startIndexLocation, startVertexLocation, startInstanceLocation);
        return 0;
    }
    RESULT GraphicsCommandList::CopyBufferRegion(uint32_t srcOffset, uint32_t dstOffset, uint32_t size, Buffer srcBuffer, Buffer dstBuffer)
    {
        VkBufferCopy copy{};
        copy.size = size;
        copy.srcOffset = srcOffset;
        copy.dstOffset = dstOffset;
        vkCmdCopyBuffer((VkCommandBuffer)ID, (VkBuffer)srcBuffer.ID, (VkBuffer)dstBuffer.ID, 1, &copy);
        return RESULT();
    }
    RESULT GraphicsCommandList::CopyBufferToImage(uint32_t srcOffset, uint32_t srcRowWidth, uint32_t srcHeight, SubResourceRange dstRange, Offset3D imgOffset, Extent3D imgSize, Buffer* buffer, Texture texture)
    {
        VkBufferImageCopy copy{};
        copy.bufferImageHeight = srcHeight;
        copy.bufferOffset = srcOffset;
        copy.bufferRowLength = srcRowWidth;
        copy.imageExtent = { imgSize.width, imgSize.height, imgSize.depth };
        copy.imageOffset = { imgOffset.width, imgOffset.height, imgOffset.depth };
        copy.imageSubresource.aspectMask = (VkImageAspectFlags)dstRange.imageAspect;
        copy.imageSubresource.baseArrayLayer = dstRange.FirstArraySlice;
        copy.imageSubresource.layerCount = dstRange.NumArraySlices;
        copy.imageSubresource.mipLevel = dstRange.IndexOrFirstMipLevel;
        vkCmdCopyBufferToImage((VkCommandBuffer)ID, (VkBuffer)buffer.ID, (VkImage)texture.ID, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        return RESULT();
    }
}
