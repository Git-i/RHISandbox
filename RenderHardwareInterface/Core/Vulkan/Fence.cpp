#include "pch.h"
#include "../Fence.h"
#include "volk.h"
namespace RHI
{
	void Fence::Destroy()
	{

	}
	void Fence::Wait(std::uint64_t val)
	{
		std::uint64_t curr_val;
		vkGetSemaphoreCounterValueKHR((VkDevice)Device_ID, (VkSemaphore)ID, &curr_val);
		if (curr_val >= val)
			return;
		const uint64_t waitValue = val;
		VkSemaphoreWaitInfo waitInfo;
		waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		waitInfo.pNext = NULL;
		waitInfo.flags = 0;
		waitInfo.semaphoreCount = 1;
		waitInfo.pSemaphores = (VkSemaphore*)&ID;
		waitInfo.pValues = &waitValue;

		vkWaitSemaphoresKHR((VkDevice)Device_ID, &waitInfo, UINT64_MAX);
	}
}