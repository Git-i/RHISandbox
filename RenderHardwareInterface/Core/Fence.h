#pragma once
#include "Object.h"

namespace RHI
{
	class RHI_API Fence : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(Fence);
	public:
		void Wait(std::uint64_t val);
	};
}