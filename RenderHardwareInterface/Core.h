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



#define DECL_STRUCT_CONSTRUCTORS(x) x(){}; x(Default_t); x(Zero_t);
#define DECL_CLASS_CONSTRUCTORS(x) x() = default;x(const x&) = default;x(x&&) = default;

