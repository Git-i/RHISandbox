#include "pch.h"
#include "../Instance.h"
#include "Core.h"
#include <dxgi1_6.h>
#include "include\d3d12.h"
#include "D3D12Specific.h"
#pragma comment(lib, "dxgi.lib")
extern "C"
{
	RESULT RHI_API RHICreateInstance(RHI::Instance** instance)
	{
		RHI::D3D12Instance* d3d12instance = new RHI::D3D12Instance;
		ID3D12Debug5* pDebug;

		D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
		pDebug->EnableDebugLayer();
		pDebug->SetEnableGPUBasedValidation(true);

		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(((IDXGIFactory**)&d3d12instance->ID)));
		*instance = d3d12instance;
		return 0;
	}
}
namespace RHI
{
	
	RESULT Instance::GetPhysicalDevice(int id, PhysicalDevice** device)
	{
		*device = new PhysicalDevice;
		return ((IDXGIFactory*)ID)->EnumAdapters(id, (IDXGIAdapter**)&(*device)->ID);
	}
	RESULT Instance::CreateSwapChain(SwapChainDesc* desc,PhysicalDevice*, Device* pDevice, CommandQueue* pCommandQueue, SwapChain** pSwapChaim)
	{	
		*pSwapChaim = new D3D12SwapChain;
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
		return ((IDXGIFactory*)ID)->CreateSwapChain((IUnknown*)pCommandQueue->ID, &dxgi_desc, (IDXGISwapChain**)&(*pSwapChaim)->ID);
	}
}