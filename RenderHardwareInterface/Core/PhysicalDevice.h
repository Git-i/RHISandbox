#pragma once
#include "Core.h"
namespace RHI
{
	struct PhysicalDeviceDesc
	{
		wchar_t  Description[128];
		unsigned int   VendorId;
		unsigned int   DeviceId;
		unsigned int   SubSysId;
		unsigned int   Revision;
		SIZE_T DedicatedVideoMemory;
		SIZE_T DedicatedSystemMemory;
		SIZE_T SharedSystemMemory;
		LUID   AdapterLuid;
	};
	class RHI_API PhysicalDevice
	{
	public:
		RESULT GetDesc(PhysicalDeviceDesc* desc);
		Internal_ID ID;;
	};

}
