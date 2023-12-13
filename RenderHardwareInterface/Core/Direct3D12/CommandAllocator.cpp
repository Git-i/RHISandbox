#include "pch.h"
#include "../CommandAllocator.h"
#include "include/d3d12.h"
namespace RHI
{
	RESULT RHI::CommandAllocator::Reset()
	{	
		((ID3D12CommandAllocator*)ID)->Reset();
		return 0;
	}
	CommandAllocator::~CommandAllocator()
	{
	}
}
