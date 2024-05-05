#pragma once
#include "Object.h"
#include "FormatsAndTypes.h"

namespace RHI
{
	class RHI_API DebugBuffer : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(DebugBuffer);
	public:
		uint32_t GetValue();
	};
}
