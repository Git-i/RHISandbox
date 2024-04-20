#include "pch.h"
#include "../Object.h"
#include "VulkanSpecific.h"

namespace RHI {
	int Object::Hold()
	{
		*refCnt += 1;
		return *refCnt;
	}
	int Object::Release()
	{
		*refCnt -= 1;
		if (*refCnt <= 0) {
			this->Destroy(); 
			return 0;
		}
		return *refCnt;
	}
	int Object::GetRefCount()
	{
		return *refCnt;
	}
}