#pragma once
#include "Core.h"
#include "Core/PhysicalDevice.h"
#include "SwapChain.h"
#include "Device.h"
namespace RHI
{
	class RHI_API Instance
	{
	public:
		static RESULT Create(Instance* instance);
		RESULT GetPhysicalDevice(int id, PhysicalDevice* device);
		RESULT CreateSwapChain(SwapChainDesc* desc, PhysicalDevice device, Device pDevice, CommandQueue pCommandQueue, SwapChain* pSwapChain);
		Internal_ID ID;
	};

}
