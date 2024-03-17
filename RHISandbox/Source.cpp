#define UNICODE
#include "Core/Instance.h"
#include "Core/Device.h"
#include "iostream"
#include <vector>
#include <DirectXMath.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <chrono>
#pragma comment(lib, "d3d12.lib")

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 611; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
const std::uint32_t STAGING_BUFFER_SIZE = 2'097'152; //2MB staging buffer

struct Vertex
{
	struct { float x, y, z; } Position;
	struct { float x, y, z; } Color;
};
struct Constants
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 _pad;

	DirectX::XMFLOAT4X4 _model;
	DirectX::XMFLOAT4X4 _view;
	DirectX::XMFLOAT4X4 _projection;
	DirectX::XMFLOAT4X4 __pad;

	DirectX::XMFLOAT4X4 __model;
	DirectX::XMFLOAT4X4 __view;
	DirectX::XMFLOAT4X4 __projection;
	DirectX::XMFLOAT4X4 ___pad;

	DirectX::XMFLOAT4X4 ___model;
	DirectX::XMFLOAT4X4 ___view;
	DirectX::XMFLOAT4X4 ___projection;
	DirectX::XMFLOAT4X4 ____pad;
};
LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CLOSE)
	{
		DestroyWindow(hWnd);
	}
	if (uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);
	}
	if (uMsg == WM_SIZE)
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
HWND CreateWin32Window()
{
	WNDCLASSEX wndclass = {};
	wndclass.cbSize = sizeof(wndclass);
	wndclass.hInstance = GetModuleHandleA(NULL);
	wndclass.lpszClassName = L"Window";
	wndclass.lpfnWndProc = WndProc;
	RegisterClassEx(&wndclass);
	RECT winRect;
	winRect.left = 0;
	winRect.right = 1280;
	winRect.top = 0;
	winRect.bottom = 720;
	AdjustWindowRectEx(&winRect, WS_OVERLAPPEDWINDOW, NULL, NULL);
	HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, L"Window", L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, winRect.right - winRect.left, winRect.bottom - winRect.top, NULL, NULL, GetModuleHandleA(NULL), NULL);
	ShowWindow(hwnd, SW_SHOW);
	return hwnd;
}
void InitializeRHI(RHI::Instance** inst, RHI::PhysicalDevice** phys_device, RHI::Device** device, RHI::CommandQueue** queue,RHI::SwapChain** swapChain, RHI::Texture** backBufferImages, RHI::Texture** depthBufferImage, RHI::DescriptorHeap** rtvHeap,RHI::DescriptorHeap** dsvHeap, HWND hwnd)
{
	RHI::CommandQueueDesc cmdDesc;
	cmdDesc.CommandListType = RHI::CommandListType::Direct;
	cmdDesc.Priority = 0.f;

	RHICreateInstance(inst);
	(*inst)->GetPhysicalDevice(0, phys_device);
	RHI::PhysicalDeviceDesc desc;
	(*phys_device)->GetDesc(&desc);
	std::wcout << desc.Description << std::endl;
	RHICreateDevice(*phys_device, &cmdDesc, 1, queue, (*inst)->ID, device);

	RHI::Surface surface;
	surface.InitWin32(hwnd, (*inst)->ID);
	RHI::SwapChainDesc sDesc(RHI::Default);
	sDesc.OutputSurface = surface;
	sDesc.Width = 1280;
	sDesc.Height = 720;
	sDesc.SwapChainFormat = RHI::Format::B8G8R8A8_UNORM;

	(*inst)->CreateSwapChain(&sDesc, *phys_device, *device, *queue, swapChain);
	(*device)->GetSwapChainImage(*swapChain, 0, &backBufferImages[0]);
	(*device)->GetSwapChainImage(*swapChain, 1, &backBufferImages[1]);

	RHI::PoolSize pSize;
	pSize.numDescriptors = 2;
	pSize.type = RHI::DescriptorType::RTV;
	RHI::DescriptorHeapDesc rtvHeapHesc;
	rtvHeapHesc.maxDescriptorSets = 1;
	rtvHeapHesc.numPoolSizes = 1;
	rtvHeapHesc.poolSizes = &pSize;
	(*device)->CreateDescriptorHeap(&rtvHeapHesc, rtvHeap);

	RHI::ClearValue depthVal = {};
	depthVal.format = RHI::Format::D32_FLOAT;
	depthVal.depthStencilValue.depth = 1.f;
	RHI::TextureDesc depthTexture;
	depthTexture.depthOrArraySize = 1;
	depthTexture.format = RHI::Format::D32_FLOAT;
	depthTexture.height = 720;
	depthTexture.mipLevels = 1;
	depthTexture.mode = RHI::TextureTilingMode::Optimal;
	depthTexture.sampleCount = 1;
	depthTexture.type = RHI::TextureType::Texture2D;
	depthTexture.usage = RHI::TextureUsage::DepthStencilAttachment;
	depthTexture.width = 1280;
	depthTexture.optimizedClearValue = &depthVal;
	RHI::HeapProperties props;
	props.type = RHI::HeapType::Default;
	(*device)->CreateTexture(&depthTexture, depthBufferImage, nullptr,&props,0, 0, RHI::ResourceType::Commited);
	pSize.type = RHI::DescriptorType::DSV;
	pSize.numDescriptors = 1;
	(*device)->CreateDescriptorHeap(&rtvHeapHesc, dsvHeap);
}
int main()
{
	HWND hwnd = CreateWin32Window();

	MSG msg = {};

	RHI::Instance* inst = {};
	RHI::Device* device;
	RHI::PhysicalDevice* phys_device;
	RHI::CommandQueue* commandQueue;
	RHI::SwapChain* swapChain = {};
	RHI::Texture* backBufferImages[2] = {};
	RHI::Texture* depthStencilView;
	RHI::DescriptorHeap* rtvDescriptorHeap;
	RHI::DescriptorHeap* dsvDescriptorHeap;

	RHI::Texture* texture;
	RHI::TextureView* textureView;
	RHI::Texture* Depthtexture;
	RHI::ComputePipeline* pipeline;
	RHI::RootSignature* rSig;
	RHI::DescriptorSetLayout* dsLayout;
	RHI::DescriptorHeap* dHeap;
	RHI::DescriptorSet* dSet;
	RHI::Fence* fence;

	InitializeRHI(&inst, &phys_device, &device, &commandQueue, &swapChain, backBufferImages, &Depthtexture, &rtvDescriptorHeap, &dsvDescriptorHeap, hwnd);
	struct FrameResource
	{
		std::uint64_t fence_val = 0;
		RHI::CommandAllocator* commandAllocator;
		int Release() { return commandAllocator->Release(); };
	};
	RHI::GraphicsCommandList* commandList;
	FrameResource fResources[3];
	for (auto& s : fResources)
	{
		device->CreateCommandAllocators(RHI::CommandListType::Direct, 1, &s.commandAllocator);
	}
	device->CreateFence(&fence, 0);
	device->CreateCommandList(RHI::CommandListType::Direct, fResources[1].commandAllocator, &commandList);
	RHI::TextureDesc tdesc;
	tdesc.depthOrArraySize = 1;
	tdesc.format = RHI::Format::R32G32B32A32_FLOAT;
	tdesc.height = 256;
	tdesc.mipLevels = 1;
	tdesc.mode = RHI::TextureTilingMode::Optimal;
	tdesc.optimizedClearValue = nullptr;
	tdesc.sampleCount = 1;
	tdesc.type = RHI::TextureType::Texture2D;
	tdesc.usage = RHI::TextureUsage::StorageImage;
	tdesc.width = 256;
	RHI::AutomaticAllocationInfo info;
	info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
	device->CreateTexture(&tdesc, &texture,0,0,&info,0,RHI::ResourceType::Automatic);
	RHI::DescriptorRange range;
	range.BaseShaderRegister = 0;
	range.numDescriptors = 1;
	range.stage = RHI::ShaderStage::Compute;
	range.type = RHI::DescriptorType::CSTexture;
	RHI::RootParameterDesc rpDesc;
	rpDesc.type = RHI::RootParameterType::DescriptorTable;
	rpDesc.descriptorTable.setIndex = 0;
	rpDesc.descriptorTable.numDescriptorRanges = 1;
	rpDesc.descriptorTable.ranges = &range;
	RHI::RootSignatureDesc rsDesc;
	rsDesc.numRootParameters = 1;
	rsDesc.rootParameters = &rpDesc;
	device->CreateRootSignature(&rsDesc, &rSig, &dsLayout);

	RHI::ComputePipelineDesc cpDesc;
	cpDesc.CS = { (char*)"shaders/ComputeShader",0 };
	cpDesc.mode = RHI::ShaderMode::File;
	cpDesc.rootSig = rSig;
	device->CreateComputePipeline(&cpDesc, &pipeline);
	RHI::PoolSize pSize;
	pSize.numDescriptors = 7;
	pSize.type = RHI::DescriptorType::CSTexture;
	RHI::DescriptorHeapDesc dhDesc;
	dhDesc.maxDescriptorSets = 10;
	dhDesc.numPoolSizes = 1;
	dhDesc.poolSizes = &pSize;
	device->CreateDescriptorHeap(&dhDesc, &dHeap);
	device->CreateDescriptorSets(dHeap, 1, dsLayout, &dSet);
	RHI::TextureViewDesc tvDesc;
	tvDesc.format = RHI::Format::R32G32B32A32_FLOAT;
	tvDesc.range =
	{
		.imageAspect = RHI::Aspect::COLOR_BIT,
		.IndexOrFirstMipLevel = 0,
		.NumMipLevels = 1,
		.FirstArraySlice = 0,
		.NumArraySlices = 1,
	};
	tvDesc.type = RHI::TextureViewType::Texture2D;
	tvDesc.texture = texture;
	device->CreateTextureView(&tvDesc, &textureView);
	RHI::DescriptorSetUpdateDesc dsuDesc;
	dsuDesc.arrayIndex = 0;
	dsuDesc.binding = 0;
	dsuDesc.numDescriptors = 1;
	dsuDesc.type = RHI::DescriptorType::CSTexture;
	RHI::DescriptorTextureInfo tinfo;
	tinfo.texture = textureView;
	dsuDesc.textureInfos = &tinfo;
	device->UpdateDescriptorSets(1, &dsuDesc, dSet);
	uint64_t fenceVal = 0;
	uint32_t currentFrame = 0;
	uint32_t framesInFlight = 3;
	uint32_t currentRtvIndex = 0;
	bool running = true;
	while (running)
	{
		if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			if (msg.message == WM_QUIT)
				running = false;
		}
		currentFrame = (currentFrame + 1) % framesInFlight;

		fence->Wait(fResources[currentFrame].fence_val);
		fResources[currentFrame].commandAllocator->Reset();
		fenceVal++;
		RHI::TextureMemoryBarrier barr;
		barr.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::NONE;
		barr.oldLayout = RHI::ResourceLayout::UNDEFINED;
		barr.newLayout = RHI::ResourceLayout::PRESENT;

		barr.previousQueue = RHI::QueueFamily::Ignored;
		barr.nextQueue = RHI::QueueFamily::Ignored;
		barr.subresourceRange = {
			.imageAspect = RHI::Aspect::COLOR_BIT,
			.IndexOrFirstMipLevel = 0,
			.NumMipLevels = 1,
			.FirstArraySlice = 0,
			.NumArraySlices = 1,
		};
		barr.texture = backBufferImages[currentRtvIndex];
		RHI::TextureMemoryBarrier barr2;
		barr2.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
		barr2.AccessFlagsAfter = RHI::ResourceAcessFlags::SHADER_WRITE;
		barr2.oldLayout = RHI::ResourceLayout::UNDEFINED;
		barr2.newLayout = RHI::ResourceLayout::GENERAL;
		barr2.previousQueue = RHI::QueueFamily::Ignored;
		barr2.nextQueue = RHI::QueueFamily::Ignored;
		barr2.subresourceRange = {
			.imageAspect = RHI::Aspect::COLOR_BIT,
			.IndexOrFirstMipLevel = 0,
			.NumMipLevels = 1,
			.FirstArraySlice = 0,
			.NumArraySlices = 1,
		};
		barr2.texture = texture;
		commandList->Begin(fResources[currentFrame].commandAllocator);
		commandList->SetComputePipeline(pipeline);
		commandList->BindComputeDescriptorSet(rSig, dSet, 0);
		commandList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::COMPUTE_SHADER_BIT, 0, 0, 1, &barr2);
		commandList->Dispatch(256, 256, 1);
		commandList->PipelineBarrier(RHI::PipelineStage::COMPUTE_SHADER_BIT, RHI::PipelineStage::BOTTOM_OF_PIPE_BIT, 0, 0, 1, &barr);
		commandList->End();

		// Submit the command buffer to the queue
		Internal_ID ids[1] = { commandList->ID };
		commandQueue->ExecuteCommandLists(ids, 1);

		fResources[currentFrame].fence_val = ++fenceVal;
		commandQueue->SignalFence(fence, fenceVal);

		swapChain->Present(currentRtvIndex);

		currentRtvIndex = (currentRtvIndex + 1) % 2;
	}
	
	
}