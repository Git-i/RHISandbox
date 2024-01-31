#include "pch.h"
#include "../CommandAllocator.h"
#include "volk.h"
#include "VulkanSpecific.h"
namespace RHI
{
	RESULT RHI::CommandAllocator::Reset()
	{	
		//transient buffers only
		if (((vCommandAllocator*)this)->m_pools.size())
		{
			vkFreeCommandBuffers( (VkDevice) ((vDevice*)device)->ID, (VkCommandPool)ID, ((vCommandAllocator*)this)->m_pools.size(), (VkCommandBuffer*)((vCommandAllocator*)this)->m_pools.data());
			((vCommandAllocator*)this)->m_pools.clear();
		}
		return vkResetCommandPool((VkDevice)((vDevice*)device)->ID, (VkCommandPool)ID, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
}
