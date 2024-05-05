#include "pch.h"
#include "../Fence.h"
#include "include/d3d12.h"
namespace RHI
{
	void Fence::Wait(std::uint64_t val)
	{
		if (((ID3D12Fence*)ID)->GetCompletedValue() >= val)
			return;
		HANDLE eventHandle = CreateEventEx(nullptr, NULL, false, EVENT_ALL_ACCESS);
		((ID3D12Fence*)ID)->SetEventOnCompletion(val, eventHandle);
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}