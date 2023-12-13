#pragma once
#include "Core.h"
#include "FormatsAndTypes.h"
#include "Texture.h"
namespace RHI
{
	struct RHI_API TextureMemoryBarrierStorage
	{
	private:
		char bytes[TEXTURE_MEMORY_BARRIER_DEVICE_SIZE];
	};
	struct RHI_API BufferMemoryBarrierStorage
	{
	private:
		char bytes[BUFFER_MEMORY_BARRIER_DEVICE_SIZE];
	};
	struct RHI_API TextureMemoryBarrier
	{
		DECL_STRUCT_CONSTRUCTORS(TextureMemoryBarrier);
		ResourceAcessFlags         AccessFlagsBefore;
		ResourceAcessFlags         AccessFlagsAfter;
		ResourceLayout              oldLayout;
		ResourceLayout             newLayout;
		Texture                   texture;
		SubResourceRange   subresourceRange;
	};
	struct RHI_API BufferMemoryBarrier
	{
		//DECL_STRUCT_CONSTRUCTORS(BufferMemoryBarrier);
	};
	TextureMemoryBarrierStorage RHI_API ConvertToDeviceFormat(TextureMemoryBarrier* desc);
	BufferMemoryBarrierStorage RHI_API ConvertToDeviceFormat(BufferMemoryBarrier* desc);
}