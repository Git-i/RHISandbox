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
	class RHI_API Resource : public Object
	{
	protected:
		friend class Device;
		std::uint64_t offset;
		std::uint64_t size;
		Internal_ID heap;
	public:
		RESULT Map(void** data);
		RESULT UnMap();
	};
}