#include "pch.h"
#include "../SwapChain.h"
#include "volk.h"
#include "VulkanSpecific.h"
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
		
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 0;
		presentInfo.pWaitSemaphores = &((vSwapChain*)this)->present_semaphore;
		VkSwapchainKHR swapChains[] = { (VkSwapchainKHR)ID };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIndex;
		vkQueuePresentKHR(((vSwapChain*)this)->PresentQueue_ID, &presentInfo);
		return RESULT();
	}
	RESULT SwapChain::AcquireImage(std::uint32_t imgIndex)
	{
		vkAcquireNextImageKHR((VkDevice)((vDevice*)device)->ID, (VkSwapchainKHR)ID, UINT64_MAX, ((vSwapChain*)this)->present_semaphore, VK_NULL_HANDLE, &imgIndex);
		VkSubmitInfo info{};
		info.pWaitSemaphores = &((vSwapChain*)this)->present_semaphore;
		info.waitSemaphoreCount = 1;
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		vkQueueSubmit(((vSwapChain*)this)->PresentQueue_ID, 1, &info, 0);
		return 0;
	}
}
