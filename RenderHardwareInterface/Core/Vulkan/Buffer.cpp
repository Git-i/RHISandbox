#include "pch.h"
#include "VulkanSpecific.h"
#include "../Buffer.h"
#include "volk.h"
namespace RHI
{
	RESULT Buffer::Map(void** data)
	{
		auto vbuffer = (vBuffer*)this;
		if (vbuffer->vma_ID)
		{
			return vmaMapMemory(vma_allocator, vbuffer->vma_ID, data);
		}
		return vkMapMemory((VkDevice)((vDevice*)device)->ID, (VkDeviceMemory)vbuffer->heap, vbuffer->offset, vbuffer->size, 0, data);
	}
	RESULT Buffer::UnMap()
	{
		auto vbuffer = (vBuffer*)this;
		if (vbuffer->vma_ID)
		{
			vmaUnmapMemory(vma_allocator, vbuffer->vma_ID);
			return 0;
		}
		vkUnmapMemory((VkDevice)((vDevice*)device)->ID, (VkDeviceMemory)vbuffer->heap);
		return 0;
	}
}

