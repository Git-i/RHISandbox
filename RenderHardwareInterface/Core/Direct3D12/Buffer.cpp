#include "pch.h"
#include "../Buffer.h"
#include "include/d3d12.h"
namespace RHI
{
	RESULT Resource::Map(void** data)
	{
		return ((ID3D12Resource*)ID)->Map(0, nullptr, data);
	}
	RESULT Resource::UnMap()
	{
		((ID3D12Resource*)ID)->Unmap(0, 0);
		return 0;
	}
}
