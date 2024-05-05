#pragma once
#include "Core.h"
namespace RHI
{
	struct LUID
	{
		uint8_t data[8] = {};
	};
	struct PhysicalDeviceDesc
	{
		wchar_t  Description[128];
		SIZE_T DedicatedVideoMemory;
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
