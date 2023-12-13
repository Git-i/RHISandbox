#include "pch.h"
#include "../SwapChain.h"
#include "volk.h"
#include <vector>
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
		vkAcquireNextImageKHR((VkDevice)Device_ID, (VkSwapchainKHR)ID, UINT64_MAX, (VkSemaphore)present_semaphore, VK_NULL_HANDLE, &imgIndex);
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = (VkSemaphore*)&present_semaphore;
		VkSwapchainKHR swapChains[] = { (VkSwapchainKHR)ID };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIndex;
		vkQueuePresentKHR((VkQueue)PresentQueue_ID, &presentInfo);
		return RESULT();
	}
	void SwapChain::Destroy()
	{

	}
}
