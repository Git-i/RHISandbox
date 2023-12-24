#include "pch.h"
#include "../CommandAllocator.h"
#include "volk.h"
namespace RHI
{
	RESULT RHI::CommandAllocator::Reset()
	{	
		return vkResetCommandPool((VkDevice)Device_ID, (VkCommandPool)ID, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
}
