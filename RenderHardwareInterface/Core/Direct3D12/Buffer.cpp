#include "pch.h"
#include "../Buffer.h"
#include "include/d3d12.h"
namespace RHI
{
	RESULT Buffer::Map(void** data)
	{
		return ((ID3D12Resource*)ID)->Map(0, nullptr, data);
	}
	RESULT Buffer::UnMap()
	{
		((ID3D12Resource*)ID)->Unmap(0, 0);
		return 0;
	}
}
