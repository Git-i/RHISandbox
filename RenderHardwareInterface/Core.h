#pragma once
#ifdef RHI_DLL
#define RHI_API __declspec(dllexport)
#else
#define RHI_API __declspec(dllimport)
#endif // RHI_DLL
typedef void* Internal_ID;
typedef unsigned int RESULT;

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#define VK_USE_PLATFORM_WIN32_KHR
	#include <Windows.h>
#endif


#include <cstdint>

#ifdef RHI_VULKAN
	#define DESTROY_FN virtual void Destroy() override;
	#define VK_NO_PROTOTYPES
#else
	#define DESTROY_FN
#endif // RHI_VULKAN

#define DECL_STRUCT_CONSTRUCTORS(x) x(); x(Default_t); x(Zero_t);
#include "Core/DeviceFormatSizes.h"


