#pragma once
#include "Object.h"
#include "FormatsAndTypes.h"

namespace RHI
{
	class RHI_API CommandAllocator : public Object
	{
	public:
		RESULT Reset();
		~CommandAllocator();
		DESTROY_FN;
	};
	
}
