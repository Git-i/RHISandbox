#include "pch.h"
#include "../PhysicalDevice.h"
#include <dxgi1_6.h>
namespace RHI
{
    RESULT PhysicalDevice::GetDesc(PhysicalDeviceDesc* returnVal)
    {
		DXGI_ADAPTER_DESC desc;
		RESULT result = ((IDXGIAdapter*)ID)->GetDesc(&desc);
		returnVal->AdapterLuid = desc.AdapterLuid;
		returnVal->DedicatedSystemMemory = desc.DedicatedSystemMemory;
		returnVal->DedicatedVideoMemory = desc.DedicatedVideoMemory;
		returnVal->DeviceId = desc.DeviceId;
		returnVal->Revision = desc.Revision;
		returnVal->SharedSystemMemory = desc.SharedSystemMemory;
		returnVal->SubSysId = desc.SubSysId;
		returnVal->VendorId = desc.VendorId;
		wcscpy(returnVal->Description, desc.Description);
		return result;
    }
}
