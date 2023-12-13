#include "pch.h"
#include "../Barrier.h"
#include "volk.h"
namespace RHI
{
	RHI::TextureMemoryBarrier::TextureMemoryBarrier()
	{
	}
	RHI::TextureMemoryBarrier::TextureMemoryBarrier(Default_t)
	{
	}
	RHI::TextureMemoryBarrier::TextureMemoryBarrier(Zero_t)
	{
	}
	TextureMemoryBarrierStorage RHI_API ConvertToDeviceFormat(TextureMemoryBarrier* desc)
	{
		VkImageMemoryBarrier barr = {};
		barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barr.image = (VkImage)desc->texture.ID;
		barr.newLayout = (VkImageLayout)desc->newLayout;
		barr.oldLayout = (VkImageLayout)desc->oldLayout;
		barr.srcAccessMask = (VkAccessFlags)desc->AccessFlagsBefore;
		barr.dstAccessMask = (VkAccessFlags)desc->AccessFlagsAfter;
		barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		VkImageSubresourceRange range;
		range.aspectMask = (VkImageAspectFlags)desc->subresourceRange.imageAspect;
		range.baseMipLevel = desc->subresourceRange.IndexOrFirstMipLevel;
		range.baseArrayLayer = desc->subresourceRange.FirstArraySlice;
		range.layerCount = desc->subresourceRange.NumArraySlices;
		range.levelCount = desc->subresourceRange.NumMipLevels;
		barr.subresourceRange = range;
		return *(TextureMemoryBarrierStorage*)(&barr);
	}
	BufferMemoryBarrierStorage RHI_API ConvertToDeviceFormat(BufferMemoryBarrier* desc)
	{
		return BufferMemoryBarrierStorage();
	}
	
}
