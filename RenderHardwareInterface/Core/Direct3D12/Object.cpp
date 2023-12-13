#include "pch.h"
#include "../Object.h"
#include <d3d12.h>
namespace RHI {
	int Object::Hold()
	{
		return ((ID3D12Object*)ID)->AddRef();
	}
	int Object::Release()
	{
		return ((ID3D12Object*)ID)->Release();
	}
	int Object::GetRefCount()
	{
		((ID3D12Object*)ID)->AddRef();
		return ((ID3D12Object*)ID)->Release();
	}
}