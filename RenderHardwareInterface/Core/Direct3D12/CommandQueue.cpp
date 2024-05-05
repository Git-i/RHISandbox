#include "pch.h"
#include "../CommandQueue.h"
#include "include/d3d12.h"
namespace RHI
{
	RESULT CommandQueue::SignalFence(Fence* fence, std::uint64_t val)
	{
		return ((ID3D12CommandQueue*)ID)->Signal((ID3D12Fence*)fence->ID, val);
	}
	RESULT CommandQueue::ExecuteCommandLists(const Internal_ID* lists, std::uint32_t count)
	{
		((ID3D12CommandQueue*)ID)->ExecuteCommandLists(count, (ID3D12CommandList**)lists);
		return 0;
	}
	RESULT CommandQueue::WaitForFence(Fence*, std::uint64_t val)
	{
		return RESULT();
	}
	
	

}