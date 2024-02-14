#include "pch.h"
#include "../FormatUtils.h"
namespace RHI
{
	namespace Util
	{
		uint32_t GetFormatBPP(RHI::Format format)
		{
			//TODO
			if (format == Format::R32G32B32A32_FLOAT) return 16;
			if (format == Format::R16G16B16A16_FLOAT) return 8;
			return 4;
		}
	}
}
