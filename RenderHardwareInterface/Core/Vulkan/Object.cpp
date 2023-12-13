#include "pch.h"
#include "../Object.h"
#include "VulkanSpecific.h"

namespace RHI {
	int Object::Hold()
	{
		std::uint64_t cnt;
		GetPrivateData((VkDevice)Device_ID, VkObjectType::VK_OBJECT_TYPE_UNKNOWN, (std::uint64_t)ID, (VkPrivateDataSlot)PrivateDataSlot, &cnt);
		cnt++;
		SetPrivateData((VkDevice)Device_ID, VkObjectType::VK_OBJECT_TYPE_UNKNOWN, (std::uint64_t)ID, (VkPrivateDataSlot)PrivateDataSlot, cnt);
		return cnt;
	}
	int Object::Release()
	{
		std::uint64_t cnt;
		GetPrivateData((VkDevice)Device_ID, VkObjectType::VK_OBJECT_TYPE_UNKNOWN, (std::uint64_t)ID, (VkPrivateDataSlot)PrivateDataSlot, &cnt);
		cnt--;
		if (cnt <= 0)
			Destroy();
		SetPrivateData((VkDevice)Device_ID, VkObjectType::VK_OBJECT_TYPE_UNKNOWN, (std::uint64_t)ID, (VkPrivateDataSlot)PrivateDataSlot, cnt);
		return cnt;
	}
	int Object::GetRefCount()
	{
		std::uint64_t cnt;
		GetPrivateData((VkDevice)Device_ID, VkObjectType::VK_OBJECT_TYPE_UNKNOWN, (std::uint64_t)ID, (VkPrivateDataSlot)PrivateDataSlot, &cnt);
		return cnt;
	}
}