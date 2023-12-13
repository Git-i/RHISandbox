#include "pch.h"
#include "../PhysicalDevice.h"
#include "volk.h"
#include <string>
namespace RHI
{
    RESULT PhysicalDevice::GetDesc(PhysicalDeviceDesc* returnVal)
    {
		VkPhysicalDeviceProperties result;
		vkGetPhysicalDeviceProperties((VkPhysicalDevice)ID, &result);
		returnVal->DeviceId = result.vendorID;
		returnVal->VendorId = result.vendorID;
		mbstowcs(returnVal->Description, result.deviceName, 128);
		VkPhysicalDeviceMemoryProperties props;
		vkGetPhysicalDeviceMemoryProperties((VkPhysicalDevice)ID, &props);
		return 0;
    }
}
