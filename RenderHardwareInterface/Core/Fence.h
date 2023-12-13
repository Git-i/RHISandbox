#pragma once
#include "Object.h"

namespace RHI
{
	class RHI_API Fence : public Object
	{
	public:
		void Wait(std::uint64_t val);
		DESTROY_FN;
	};
}