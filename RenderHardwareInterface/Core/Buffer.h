#pragma once
#include "Object.h"
#include "FormatsAndTypes.h"
namespace RHI
{
	class RHI_API Buffer :public Resource
	{
		
	};
	struct RHI_API BufferDesc
	{
		std::uint32_t size;
		BufferUsage usage;
	};
}
