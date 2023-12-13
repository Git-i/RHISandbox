#include "pch.h"
#include "../Surface.h"
#include "volk.h"
namespace RHI
{
	void RHI::Surface::InitWin32(HWND hwnd, Internal_ID instance)
	{
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = hwnd;
		createInfo.hinstance = GetModuleHandle(nullptr);
		vkCreateWin32SurfaceKHR((VkInstance)instance, &createInfo, nullptr, (VkSurfaceKHR*)&ID);
	}
}