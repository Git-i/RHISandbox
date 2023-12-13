#include "pch.h"
#include "../SwapChain.h"

namespace RHI
{
	SwapChainDesc::SwapChainDesc(Default_t)
	{
		RefreshRate = { 60, 1 };
		BufferCount = 2;
		Flags = 0;
		Width = 0;
		Height = 0;
		OutputSurface = Surface();
		SampleCount = 1;
		SampleQuality = 0;
		SwapChainFormat = RHI::Format::R8G8B8A8_UNORM;
		Windowed = true;
		BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	}
	SwapChainDesc::SwapChainDesc(Zero_t)
	{
		ZeroMemory(this, sizeof(SwapChainDesc));
	}
	SwapChainDesc::SwapChainDesc()
	{
	}
	RESULT SwapChain::Present(std::uint32_t imgIndex)
	{
		
		((IDXGISwapChain*)ID)->Present(0, 0);
		return RESULT();
	}
}
