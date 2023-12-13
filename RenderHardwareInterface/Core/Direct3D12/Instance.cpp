#include "pch.h"
#include "../Instance.h"
#include "Core.h"
#include <dxgi1_6.h>
#include "include\d3d12.h"
#pragma comment(lib, "dxgi.lib")
namespace RHI
{
	RESULT Instance::Create(Instance* instance)
	{
		ID3D12Debug5* pDebug;
	
		D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
		pDebug->EnableDebugLayer();
		pDebug->SetEnableGPUBasedValidation(true);

		return CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(((IDXGIFactory**)&instance->ID)));
	}
	RESULT Instance::GetPhysicalDevice(int id, PhysicalDevice* device)
	{
		return ((IDXGIFactory*)ID)->EnumAdapters(id, (IDXGIAdapter**)&device->ID);
	}
	RESULT Instance::CreateSwapChain(SwapChainDesc* desc,PhysicalDevice, Device pDevice, CommandQueue pCommandQueue, SwapChain* pSwapChaim)
	{	
		DXGI_SWAP_CHAIN_DESC dxgi_desc;
		dxgi_desc.BufferCount = desc->BufferCount;
		dxgi_desc.BufferDesc.Width = desc->Width;
		dxgi_desc.BufferDesc.Height = desc->Height;
		dxgi_desc.BufferDesc.RefreshRate.Numerator = desc->RefreshRate.Numerator;
		dxgi_desc.BufferDesc.RefreshRate.Denominator = desc->RefreshRate.Denominator;
		dxgi_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
		dxgi_desc.BufferUsage = desc->BufferUsage;
		dxgi_desc.Flags = desc->Flags;
		dxgi_desc.OutputWindow = (HWND__*)desc->OutputSurface.ID;
		dxgi_desc.SampleDesc.Count = desc->SampleCount;
		dxgi_desc.SampleDesc.Quality = desc->SampleQuality;
		dxgi_desc.Windowed = desc->Windowed;
		dxgi_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxgi_desc.BufferDesc.Format = (DXGI_FORMAT)desc->SwapChainFormat;
		return ((IDXGIFactory*)ID)->CreateSwapChain((IUnknown*)pCommandQueue.ID, &dxgi_desc, (IDXGISwapChain**)&pSwapChaim->ID);
	}
}