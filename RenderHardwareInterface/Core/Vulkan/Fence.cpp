#include "pch.h"
#include "VulkanSpecific.h"
#include "../Fence.h"
#include "volk.h"
namespace RHI
{
	void Fence::Wait(std::uint64_t val)
	{
		//std::uint64_t curr_val;
		//vkGetSemaphoreCounterValueKHR((VkDevice)((vDevice*)device)->ID, (VkSemaphore)ID, &curr_val);
		//if (curr_val >= val)
		//	return;
		const uint64_t waitValue = val;
		VkSemaphoreWaitInfo waitInfo;
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.pNext = NULL;
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = (VkSemaphore*)&ID;
		waitInfo.pValues = &waitValue;
		
		vkWaitSemaphoresKHR((VkDevice)((vDevice*)device)->ID, &waitInfo, UINT64_MAX);
	}
}