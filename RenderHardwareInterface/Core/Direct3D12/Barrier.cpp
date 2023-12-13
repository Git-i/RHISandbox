#include "pch.h"
#include "../Barrier.h"
#include "include/d3d12.h"
#include "D3D12Specific.h"
namespace RHI
{
	RHI::TextureMemoryBarrier::TextureMemoryBarrier()
	{
	}
	RHI::TextureMemoryBarrier::TextureMemoryBarrier(Default_t)
	{
	}
	RHI::TextureMemoryBarrier::TextureMemoryBarrier(Zero_t)
	{
	}
	TextureMemoryBarrierStorage RHI_API ConvertToDeviceFormat(TextureMemoryBarrier* desc)
	{
		D3D12_TEXTURE_BARRIER barr = {};
		barr.AccessBefore = D3DAcessMode(desc->AccessFlagsBefore);
		barr.AccessAfter = D3DAcessMode(desc->AccessFlagsAfter);
		barr.LayoutBefore = D3DBarrierLayout(desc->oldLayout);
		barr.LayoutAfter = D3DBarrierLayout(desc->newLayout);
		barr.pResource = (ID3D12Resource*)desc->texture.ID;
		D3D12_BARRIER_SUBRESOURCE_RANGE range;
		range.FirstArraySlice = desc->subresourceRange.FirstArraySlice;
		range.NumPlanes = 1;
		range.IndexOrFirstMipLevel = desc->subresourceRange.IndexOrFirstMipLevel;
		range.NumMipLevels = desc->subresourceRange.NumMipLevels;
		range.NumArraySlices = desc->subresourceRange.NumArraySlices;
		UINT firstPlane = 0;
		UINT numPlanes = 0;
		if ((UINT)desc->subresourceRange.imageAspect & (UINT)RHI::Aspect::STENCIL_BIT)
		{
			firstPlane = 1;
		}
		if ((UINT)desc->subresourceRange.imageAspect & (UINT)RHI::Aspect::PLANE_1_BIT)
		{
			firstPlane = 1;
		}
		if ((UINT)desc->subresourceRange.imageAspect & (UINT)RHI::Aspect::PLANE_2_BIT)
		{
			firstPlane = 2;
		}
		range.FirstPlane = firstPlane;
		
		return *(TextureMemoryBarrierStorage*)(&barr);
	}
	BufferMemoryBarrierStorage RHI_API ConvertToDeviceFormat(BufferMemoryBarrier* desc)
	{
		return BufferMemoryBarrierStorage();
	}
	
}
