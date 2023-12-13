#include "pch.h"
#include "../CommandQueue.h"
#include "volk.h"
namespace RHI
{
	RESULT CommandQueue::SignalFence(Fence fence, std::uint64_t val)
	{
		std::uint64_t fenceVal = val;
		VkTimelineSemaphoreSubmitInfo timelineInfo = {};
		timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineInfo.pNext = NULL;
		timelineInfo.signalSemaphoreValueCount = 1;
		timelineInfo.pSignalSemaphoreValues = &fenceVal;
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = &timelineInfo;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = (VkSemaphore*)&fence.ID;
		return vkQueueSubmit((VkQueue)ID, 1, &submitInfo, VK_NULL_HANDLE);
	}
	RESULT CommandQueue::ExecuteCommandLists(const Internal_ID* lists, std::uint32_t count)
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = 0;
		submitInfo.commandBufferCount = count;
		submitInfo.pCommandBuffers = (VkCommandBuffer*)lists;
		return vkQueueSubmit((VkQueue)ID, 1, &submitInfo, VK_NULL_HANDLE);
	}
	void CommandQueue::Destroy()
	{
		vkDestroyCommandPool((VkDevice)Device_ID, (VkCommandPool)ID, nullptr);
	}
}