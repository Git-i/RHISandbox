#include "pch.h"
#include "../Surface.h"
namespace RHI
{
	void RHI::Surface::InitWin32(HWND hwnd, Internal_ID instance)
	{
		ID = hwnd;
	}
}