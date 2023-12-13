#include "pch.h"
#include "../Buffer.h"
#include "volk.h"
namespace RHI
{
	RESULT Resource::Map(void** data)
	{
		return vkMapMemory((VkDevice)Device_ID, (VkDeviceMemory)heap, offset, size, 0, data);
	}
	RESULT Resource::UnMap()
	{
		vkUnmapMemory((VkDevice)Device_ID, (VkDeviceMemory)heap);
		return 0;
	}
}
