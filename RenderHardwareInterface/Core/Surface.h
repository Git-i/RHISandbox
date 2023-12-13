#pragma once
#include "Core.h"
#include "FormatsAndTypes.h"
namespace RHI
{
	class RHI_API Surface
	{
	public:
#ifdef _WIN32
		void InitWin32(HWND hwnd, Internal_ID instance);
#endif

#ifdef __ANDROID__
		void InitAndroid();
#endif

#ifdef __linux__
		void InitLinux();
#endif
		Internal_ID ID = 0;
	};
}