#include "pch.h"
#include "../DescriptorHeap.h"
#include "volk.h"
namespace RHI
{
	CPU_HANDLE RHI::DescriptorHeap::GetCpuHandle()
	{
		return { ID };
	}
	//DescriptorHeapDesc::DescriptorHeapDesc(Default_t)
	//{
	//	//NumDescriptors = 1;
	//	//type = DescriptorHeapType::RTV;
	//	//shaderVisible = 0;
	//}
	//
	//DescriptorHeapDesc::DescriptorHeapDesc(Zero_t)
	//{
	//	ZeroMemory(this, sizeof(DescriptorHeapDesc));
	//}
	
}
