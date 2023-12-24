#include "pch.h"
#include "../Object.h"
#include <d3d12.h>
namespace RHI {
	int Object::Hold()
	{
		if (ID)return ((ID3D12Object*)ID)->AddRef();
		else return 0;
	}
	int Object::Release()
	{
		if (ID)
		{
			int refCnt = ((ID3D12Object*)ID)->Release();
			//if (!refCnt) operator delete((void*)this);
		}
		else return 0;
	}
	int Object::GetRefCount()
	{
		((ID3D12Object*)ID)->AddRef();
		return ((ID3D12Object*)ID)->Release();
	}
}