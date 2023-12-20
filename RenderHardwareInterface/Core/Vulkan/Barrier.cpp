#include "pch.h"
#include "../Barrier.h"
#include "volk.h"
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
		return {};
	}
	BufferMemoryBarrierStorage RHI_API ConvertToDeviceFormat(BufferMemoryBarrier* desc)
	{
		return BufferMemoryBarrierStorage();
	}
	
}
