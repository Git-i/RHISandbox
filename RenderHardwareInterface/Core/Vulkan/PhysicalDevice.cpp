#include "pch.h"
#include "../PhysicalDevice.h"
#include "volk.h"
#include <string>
namespace RHI
{
    RESULT PhysicalDevice::GetDesc(PhysicalDeviceDesc* returnVal)
    {
		VkPhysicalDeviceIDProperties id_props{};
		id_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
		VkPhysicalDeviceProperties2 result;
		result.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		result.pNext = &id_props;
		vkGetPhysicalDeviceProperties2((VkPhysicalDevice)ID, &result);
		mbstowcs(returnVal->Description, result.properties.deviceName, 128);
		memcpy(returnVal->AdapterLuid.data, id_props.deviceLUID, sizeof(uint8_t) * 8);
		return 0;
    }
}
