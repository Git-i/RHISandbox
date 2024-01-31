#pragma once
#include "Core.h"
#include <atomic>
namespace RHI
{
	class RHI_API Object
	{
	protected:
		std::atomic<uint32_t>* refCnt;
		Object() { refCnt = new std::atomic<uint32_t>(); *refCnt = 1; }
	public:
		int Hold();
		int Release();
		int GetRefCount();
		Internal_ID ID = NULL;
		virtual void Destroy() {};
		void* device = NULL;
	};
}