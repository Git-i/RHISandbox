#pragma once
#include "Core.h"
#include "FormatsAndTypes.h"
#include "Texture.h"
#include "Buffer.h"
namespace RHI
{
	enum class QueueFamily
	{
		Graphics, Compute, Copy, Ignored
	};
	struct RHI_API TextureMemoryBarrier
	{
		DECL_STRUCT_CONSTRUCTORS(TextureMemoryBarrier);
		ResourceAcessFlags         AccessFlagsBefore;
		ResourceAcessFlags         AccessFlagsAfter;
		ResourceLayout              oldLayout;
		ResourceLayout             newLayout;
		Texture*                   texture;
		QueueFamily				   previousQueue;
		QueueFamily				   nextQueue;
		SubResourceRange   subresourceRange;
	};
	struct RHI_API BufferMemoryBarrier
	{
		DECL_STRUCT_CONSTRUCTORS(BufferMemoryBarrier);
		ResourceAcessFlags         AccessFlagsBefore;
		ResourceAcessFlags         AccessFlagsAfter;
		QueueFamily				   previousQueue;
		QueueFamily				   nextQueue;
		Buffer*                    buffer;
		uint32_t                   offset;
		uint32_t                   size;
	};
}