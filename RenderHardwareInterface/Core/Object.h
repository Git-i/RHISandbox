#pragma once
#include "Core.h"

namespace RHI
{
	class RHI_API Object
	{
	
	public:
		int Hold();
		int Release();
		int GetRefCount();
		Internal_ID ID = NULL;
		virtual void Destroy() {};
		Internal_ID Device_ID = NULL;
		Internal_ID PrivateDataSlot = NULL;
	};
}