#pragma once
#include "Core.h"
#include "PhysicalDevice.h"
#include "SwapChain.h"
#include "Device.h"
namespace RHI
{
	class RHI_API Instance : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(Instance);
	public:
		uint32_t GetNumPhysicalDevices();
		RESULT GetAllPhysicalDevices(PhysicalDevice** devices);
		RESULT GetPhysicalDevice(int id, PhysicalDevice** device);
		RESULT CreateSwapChain(SwapChainDesc* desc, PhysicalDevice* device, Device* pDevice, CommandQueue* pCommandQueue, SwapChain** pSwapChain);
	};

}
extern "C"
{
	RESULT RHI_API RHICreateInstance(RHI::Instance**instance);
}
