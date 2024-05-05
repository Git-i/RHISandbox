#include "pch.h"
#include "../Instance.h"
#include "Core.h"
#include "volk.h"
#include "VulkanSpecific.h"
#include <vector>
#include <iostream>
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::string_view view(pCallbackData->pMessage);
	char id_string[1024];
	strcpy(id_string, "MessageID = 0x");
	_itoa(pCallbackData->messageIdNumber, id_string + strlen(id_string), 16);
	size_t ind = view.find(id_string);
	ind += strlen(id_string);
	std::cerr << std::endl << "validation layer" << pCallbackData->pMessage + ind << std::endl;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT && pCallbackData->messageIdNumber != 101294395 && pCallbackData->messageIdNumber != 0x4dae5635)
	{
		__debugbreak();
	}

	return VK_FALSE;
}
extern "C"
{
	RESULT RHI_API RHICreateInstance(RHI::Instance** instance)
	{
		
		RHI::vInstance* vinstance = new RHI::vInstance;
		*instance = vinstance;
		volkInitialize();
		VkValidationFeatureEnableEXT enabled[] =
		{
			VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
		};
		VkValidationFeaturesEXT features{};
		features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		features.pEnabledValidationFeatures = enabled;
		features.enabledValidationFeatureCount = 0;
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "NULL";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "NULL";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;
		VkInstanceCreateInfo info = {};
		const char* layerNames[] = 
		{
			"VK_LAYER_KHRONOS_validation",
		};
		const char* extensionName[4] = { "VK_KHR_surface", "VK_KHR_win32_surface",VK_EXT_DEBUG_UTILS_EXTENSION_NAME,VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = &features;
		info.enabledLayerCount = ARRAYSIZE(layerNames);
		info.ppEnabledLayerNames = layerNames;
		info.enabledExtensionCount = 3;
		info.ppEnabledExtensionNames = extensionName;
		info.pApplicationInfo = &appInfo;
		VkResult res = vkCreateInstance(&info, nullptr, (VkInstance*)&vinstance->ID);
		volkLoadInstance((VkInstance)vinstance->ID);
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
		vkCreateDebugUtilsMessengerEXT((VkInstance)vinstance->ID, &createInfo, nullptr, &vinstance->messanger);
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
	API Instance::GetInstanceAPI()
	{
		return API::Vulkan;
	}
	uint32_t Instance::GetNumPhysicalDevices()
	{
		std::uint32_t count;
		vkEnumeratePhysicalDevices((VkInstance)ID, &count, nullptr);
		return count;
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

		auto [indices,_] = findQueueFamilyIndices(pDevice, desc->OutputSurface);
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