#pragma once
#include "Object.h"
#include "FormatsAndTypes.h"

namespace RHI
{
	class RHI_API CommandAllocator : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(CommandAllocator);
	public:
		RESULT Reset();
	};
	
}
