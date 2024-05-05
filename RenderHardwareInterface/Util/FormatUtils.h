#pragma once
#include "Core/FormatsAndTypes.h"
namespace RHI
{
	namespace Util
	{
		enum class FormatInfo
		{
			Color, DepthStencil
		};
		uint32_t RHI_API GetFormatBPP(RHI::Format format);
		FormatInfo RHI_API GetFormatInfo(RHI::Format format);
	}
}
