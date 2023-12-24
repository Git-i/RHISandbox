#include "pch.h"
#include "VulkanSpecific.h"
#include "../Buffer.h"
#include "volk.h"
namespace RHI
{
	RESULT Buffer::Map(void** data)
	{
		auto vbuffer = (vBuffer*)this;
		return vkMapMemory((VkDevice)Device_ID, (VkDeviceMemory)vbuffer->heap, vbuffer->offset, vbuffer->size, 0, data);
	}
	RESULT Buffer::UnMap()
	{
		auto vbuffer = (vBuffer*)this;
		vkUnmapMemory((VkDevice)Device_ID, (VkDeviceMemory)vbuffer->heap);
		return 0;
	}
}

