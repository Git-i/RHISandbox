#include "pch.h"
#include "../Instance.h"
#include "Core.h"
#include "volk.h"
#include "VulkanSpecific.h"
#include <vector>
#include <iostream>
extern "C"
{
	RESULT RHI_API RHICreateInstance(RHI::Instance** instance)
	{
		RHI::vInstance* vinstance = new RHI::vInstance;
		*instance = vinstance;
		volkInitialize();
		VkInstanceCreateInfo info = {};
		const char* layerName = "VK_LAYER_KHRONOS_validation";
		const char* extensionName[2] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = nullptr;
		info.enabledLayerCount = 1;
		info.ppEnabledLayerNames = &layerName;
		info.enabledExtensionCount = 2;
		info.ppEnabledExtensionNames = extensionName;
		VkResult res = vkCreateInstance(&info, nullptr, (VkInstance*)&vinstance->ID);
		volkLoadInstance((VkInstance)vinstance->ID);
		return res;
	}
}
namespace RHI
{
	RESULT Instance::GetPhysicalDevice(int id, PhysicalDevice** device)
	{
		vPhysicalDevice* vdevice = new vPhysicalDevice;
		std::vector<VkPhysicalDevice> devices;
		std::uint32_t count;
		vkEnumeratePhysicalDevices((VkInstance)ID, &count, nullptr);
		devices.resize(count);
		VkResult res = vkEnumeratePhysicalDevices((VkInstance)ID, &count, &devices[0]);
		vdevice->ID = devices[id];
		*device = vdevice;
		return res;
	}
	RESULT Instance::CreateSwapChain(SwapChainDesc* desc, PhysicalDevice* pDevice, Device* Device, CommandQueue* pCommandQueue, SwapChain** pSwapChain)
	{
		vSwapChain* vswapChain = new vSwapChain;
		VkSwapchainCreateInfoKHR createInfo{};
		std::uint32_t count;
		vkGetPhysicalDeviceSurfaceFormatsKHR((VkPhysicalDevice)pDevice->ID, (VkSurfaceKHR)desc->OutputSurface.ID, &count, nullptr);
		std::vector<VkSurfaceFormatKHR> format(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR((VkPhysicalDevice)pDevice->ID, (VkSurfaceKHR)desc->OutputSurface.ID, &count, format.data());

		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = (VkSurfaceKHR)desc->OutputSurface.ID;

		createInfo.minImageCount = desc->BufferCount;
		createInfo.imageFormat = FormatConv(desc->SwapChainFormat);
		createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		createInfo.imageExtent = { desc->Width, desc->Height };
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		QueueFamilyIndices indices = findQueueFamilyIndices(pDevice, desc->OutputSurface);
		uint32_t queueFamilyIndices[] = { indices.graphicsIndex, indices.presentIndex };

		if (indices.graphicsIndex != indices.presentIndex) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;
		VkResult res = vkCreateSwapchainKHR((VkDevice)Device->ID, &createInfo, nullptr, (VkSwapchainKHR*)&vswapChain->ID);
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore((VkDevice)Device->ID, &semaphoreInfo, nullptr, (VkSemaphore*)&vswapChain->present_semaphore);
		vswapChain->device = Device;
		Device->Hold();
		vkGetDeviceQueue((VkDevice)Device->ID, indices.presentIndex, 0, (VkQueue*)&vswapChain->PresentQueue_ID);
		*pSwapChain = vswapChain;
		return 0;
	}
}