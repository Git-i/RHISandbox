#include "pch.h"
#include "../DescriptorHeap.h"
#include <d3d12.h>
#include "D3D12Specific.h"
namespace RHI
{
	CPU_HANDLE RHI::DescriptorHeap::GetCpuHandle()
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = ((ID3D12DescriptorHeap*)ID)->GetCPUDescriptorHandleForHeapStart();
		CPU_HANDLE returnHandle;
		returnHandle.val = handle.ptr;
		return returnHandle;
	}
	//DescriptorHeapDesc::DescriptorHeapDesc(Default_t)
	//{
	//	NumDescriptors = 1;
	//	type = DescriptorHeapType::RTV;
	//	shaderVisible = 0;
	//}
	//
	//DescriptorHeapDesc::DescriptorHeapDesc(Zero_t)
	//{
	//	ZeroMemory(this, sizeof(DescriptorHeapDesc));
	//}
	
}
