#define UNICODE
#include "Core/Instance.h"
#include "Core/Device.h"
#include "C:\Dev\Repos\RenderHardwareInterface\packages\Microsoft.Direct3D.D3D12.1.611.2\build\native\include\d3d12.h"
#include "iostream"
#include <vector>
#include <DirectXMath.h>
#include <chrono>
#pragma comment(lib, "d3d12.lib")
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 611; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
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
void InitializeRHI(RHI::Instance* inst, RHI::PhysicalDevice* phys_device, RHI::Device* device, RHI::CommandQueue* queue, RHI::SwapChain* swapChain, RHI::Texture* backBufferImages, RHI::DescriptorHeap* rtvHeap,RHI::DescriptorHeap* dsvHeap, HWND hwnd)
{
	RHI::CommandQueueDesc cmdDesc;
	cmdDesc.CommandListType = RHI::CommandListType::Direct;
	cmdDesc.Priority = 0.f;

	RHI::Instance::Create(inst);
	inst->GetPhysicalDevice(0, phys_device);
	RHI::PhysicalDeviceDesc desc;
	phys_device->GetDesc(&desc);
	std::wcout << desc.Description << std::endl;
	RHI::Device::Create(*phys_device, &cmdDesc, 1, queue, device);

	RHI::Surface surface;
	surface.InitWin32(hwnd, inst->ID);
	RHI::SwapChainDesc sDesc(RHI::Default);
	sDesc.OutputSurface = surface;
	sDesc.Width = 1280;
	sDesc.Height = 720;
	sDesc.SwapChainFormat = RHI::Format::B8G8R8A8_UNORM;

	inst->CreateSwapChain(&sDesc, *phys_device, *device, *queue, swapChain);
	device->GetSwapChainImage(*swapChain, 0, &backBufferImages[0]);
	device->GetSwapChainImage(*swapChain, 1, &backBufferImages[1]);

	RHI::PoolSize pSize;
	pSize.numDescriptors = 2;
	pSize.type = RHI::DescriptorHeapType::RTV;
	RHI::DescriptorHeapDesc rtvHeapHesc;
	rtvHeapHesc.maxDescriptorSets = 1;
	rtvHeapHesc.numPoolSizes = 1;
	rtvHeapHesc.poolSizes = &pSize;
	device->CreateDescriptorHeap(&rtvHeapHesc, rtvHeap);
	pSize.type = RHI::DescriptorHeapType::DSV;
	pSize.numDescriptors = 1;
	//device->CreateDescriptorHeap(&rtvHeapHesc, dsvHeap);
}
int main()
{
	HWND hwnd = CreateWin32Window();
	
	MSG msg = {};

	RHI::Instance inst = {};
	RHI::Device device;
	RHI::PhysicalDevice phys_device;
	RHI::CommandQueue commandQueue;
	RHI::SwapChain swapChain = {};
	RHI::Texture backBufferImages[2] = {};
	RHI::DescriptorHeap rtvDescriptorHeap;
	RHI::DescriptorHeap dsvDescriptorHeap;
	
	RHI::Buffer VertexBuffer;
	RHI::Buffer ConstantBuffer;
	RHI::PipelineStateObject pso;
	RHI::Heap cbheap;
	RHI::Heap heap;
	RHI::RootSignature rSig;
	RHI::DescriptorSetLayout dsLayout;
	RHI::DescriptorHeap dHeap;
	RHI::DescriptorSet dSet;

	InitializeRHI(&inst, &phys_device, &device, &commandQueue, &swapChain, backBufferImages, &rtvDescriptorHeap, &dsvDescriptorHeap,hwnd);
	const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f,1.f,}},
	{{ 0.5f, -0.5f, -0.5f,}, { 1.0f, 0.0f,1.f,}},
	{{ 0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,1.f,}},
	{{ 0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,0.f,}},
	{{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,0.f,}},
	{{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,1.f,}},

	{{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,1.f,}},
	{{ 0.5f, -0.5f,  0.5f,}, { 1.0f, 0.0f,1.f,}},
	{{ 0.5f,  0.5f,  0.5f,}, { 1.0f, 1.0f,0.f,}},
	{{ 0.5f,  0.5f,  0.5f,}, { 1.0f, 1.0f,1.f,}},
	{{-0.5f,  0.5f,  0.5f,}, { 0.0f, 1.0f,0.f,}},
	{{-0.5f, -0.5f,  0.5f,}, { 0.0f, 1.0f,1.f,}},

	{{-0.5f,  0.5f,  0.5f,}, { 1.0f, 1.0f,0.f,}},
	{{-0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,1.f,}},
	{{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,0.f,}},
	{{-0.5f, -0.5f, -0.5f,}, { 0.0f, 0.0f,1.f,}},
	{{-0.5f, -0.5f,  0.5f,}, { 1.0f, 0.0f,1.f,}},
	{{-0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,0.f,}},

	{{ 0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,1.f,}},
	{{ 0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,0.f,}},
	{{ 0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,1.f,}},
	{{ 0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,0.f,}},
	{{ 0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,1.f,}},
	{{ 0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,0.f,}},

	{{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,1.f,}},
	{{ 0.5f, -0.5f, -0.5f,}, { 1.0f, 1.0f,1.f,}},
	{{ 0.5f, -0.5f,  0.5f,}, { 1.0f, 0.0f,0.f,}},
	{{ 0.5f, -0.5f,  0.5f,}, { 1.0f, 0.0f,1.f,}},
	{{-0.5f, -0.5f,  0.5f,}, { 0.0f, 0.0f,0.f,}},
	{{-0.5f, -0.5f, -0.5f,}, { 0.0f, 1.0f,1.f,}},
	
	{{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,1.f,}},
	{{ 0.5f,  0.5f, -0.5f,}, { 1.0f, 1.0f,1.f,}},
	{{ 0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,0.f,}},
	{{ 0.5f,  0.5f,  0.5f,}, { 1.0f, 0.0f,0.f,}},
	{{-0.5f,  0.5f,  0.5f,}, { 0.0f, 0.0f,0.f,}},
	{{-0.5f,  0.5f, -0.5f,}, { 0.0f, 1.0f,1.f,}},
	};
	Constants c;
	DirectX::XMStoreFloat4x4(&c.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(0.f))));
	DirectX::XMStoreFloat4x4(&c.view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(1.f, 5.f, -4.f, 1.f), DirectX::XMVectorZero(), DirectX::XMVectorSet(0, 1, 0, 0))));
	DirectX::XMStoreFloat4x4(&c.projection, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.f), 16.f / 9.f, 0.1f, 100.f)));

	DirectX::XMVECTOR v;
	v = DirectX::XMVectorSet(0, 1.f, 0, 1);
	v = DirectX::XMVector3Transform( v, DirectX::XMMatrixTranslation(0.3f, 0, 0));
	v = DirectX::XMVector3Transform( v, DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(0.f, 1.f, -3.f, 1.f), DirectX::XMVectorZero(), DirectX::XMVectorSet(0, 1, 0, 0)));
	v = DirectX::XMVector3Transform(v, DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.f), 16.f/9.f, 0.1f, 100.f));
	DirectX::XMFLOAT4 f;
	DirectX::XMStoreFloat4(&f, v);
	std::cout << f.x << " " << f.y << " " << f.z << std::endl;
	RHI::BufferDesc bufferDesc;
	bufferDesc.size = sizeof(Vertex) * vertices.size();
	bufferDesc.usage = RHI::BufferUsage::VertexBuffer;

	RHI::BufferDesc ConstantbufferDesc;
	ConstantbufferDesc.size = sizeof(Constants);
	ConstantbufferDesc.usage = RHI::BufferUsage::ConstantBuffer;

	RHI::MemoryReqirements bufferMemRequirements;
	RHI::MemoryReqirements ConstantbufferMemRequirements;
	device.GetBufferMemoryRequirements(&bufferDesc, &bufferMemRequirements);
	device.GetBufferMemoryRequirements(&ConstantbufferDesc, &ConstantbufferMemRequirements);

	RHI::HeapDesc heapDesc;
	bool fallback = true;
	heapDesc.props.type = RHI::HeapType::Custom;
	heapDesc.props.FallbackType = RHI::HeapType::Default;
	heapDesc.props.memoryLevel = RHI::MemoryLevel::DedicatedRAM;
	heapDesc.props.pageProperty = RHI::CPUPageProperty::WriteCombined;
	heapDesc.size = bufferMemRequirements.size;
	device.CreateHeap(&heapDesc, &heap, &fallback);

	heapDesc.props.memoryLevel = RHI::MemoryLevel::DedicatedRAM;
	heapDesc.props.pageProperty = RHI::CPUPageProperty::WriteCombined;
	heapDesc.size = ConstantbufferMemRequirements.size;
	device.CreateHeap(&heapDesc, &cbheap, &fallback);
	

	device.CreateBuffer(&bufferDesc, &VertexBuffer, &heap, 0, RHI::ResourceType::Placed);
	device.CreateBuffer(&ConstantbufferDesc, &ConstantBuffer, &cbheap, 0, RHI::ResourceType::Placed);
	void* data = nullptr;
	ConstantBuffer.Map(&data);
	memcpy(data, &c, sizeof(Constants));
	ConstantBuffer.UnMap();

	VertexBuffer.Map(&data);
	memcpy(data, vertices.data(), sizeof(Vertex) * vertices.size());
	VertexBuffer.UnMap();


	RHI::PipelineStateObjectDesc PSOdesc = {};
	PSOdesc.VS = "shaders/basic_triangle_vs";
	PSOdesc.PS = "shaders/basic_triangle_fs";
	PSOdesc.topology = RHI::PrimitiveTopology::TriangleList;

	RHI::InputElementDesc ied[2];
	ied[0].location = 0;
	ied[0].inputSlot = 0;
	ied[0].format = RHI::Format::R32G32B32_FLOAT;
	ied[0].alignedByteOffset = offsetof(Vertex, Position);

	ied[1].location = 1;
	ied[1].inputSlot = 0;
	ied[1].format = RHI::Format::R32G32B32_FLOAT;
	ied[1].alignedByteOffset = offsetof(Vertex, Color);

	RHI::InputBindingDesc ibd;
	ibd.inputRate = RHI::InputRate::Vertex;
	ibd.stride = sizeof(Vertex);

	RHI::DescriptorRange range;
	range.numDescriptors = 1;
	range.BaseShaderRegister = 1;

	RHI::RootParameterDesc rpDesc;
	rpDesc.type = RHI::RootParameterType::DescriptorTable;
	rpDesc.descriptorTable.numDescriptorRanges = 1;
	rpDesc.descriptorTable.ranges = &range;


	RHI::RootSignatureDesc rsDesc;
	rsDesc.numRootParameters = 1;
	rsDesc.rootParameters = &rpDesc;
	device.CreateRootSignature(&rsDesc, &rSig, &dsLayout);

	PSOdesc.inputBindings = &ibd;
	PSOdesc.inputElements = ied;
	PSOdesc.numInputBindings = 1;
	PSOdesc.numInputElements = 2;
	PSOdesc.rootSig = rSig;

	device.CreatePipelineStateObject(&PSOdesc, &pso);

	RHI::PoolSize pSize;
	pSize.numDescriptors = 1;
	pSize.type = RHI::DescriptorHeapType::SRV_CBV_UAV;

	RHI::DescriptorHeapDesc dhDesc;
	dhDesc.maxDescriptorSets = 1;
	dhDesc.numPoolSizes = 1;
	dhDesc.poolSizes = &pSize;

	device.CreateDescriptorHeap(&dhDesc, &dHeap);
	device.CreateDescriptorSets(dHeap, 1, &dsLayout, &dSet);

	RHI::DescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = ConstantBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(Constants);

	RHI::DescriptorSetUpdateDesc dhuDesc;
	dhuDesc.arrayIndex = 0;
	dhuDesc.binding = 1;
	dhuDesc.bufferInfos = &bufferInfo;
	dhuDesc.numDescriptors = 1;
	dhuDesc.type = RHI::DescriptorHeapType::SRV_CBV_UAV;

	device.UpdateDescriptorSets(1, &dhuDesc, &dSet);

	RHI::CPU_HANDLE CurrentRtvHandle = rtvDescriptorHeap.GetCpuHandle();
	RHI::CPU_HANDLE handles[2];
	for (int i = 0; i < 2; i++)
	{
		RHI::CPU_HANDLE handle;
		handle.val = CurrentRtvHandle.val + (i * device.GetDescriptorHeapIncrementSize(RHI::DescriptorHeapType::RTV));
		device.CreateRenderTargetView(backBufferImages[i], nullptr, handle);

		handles[i] = handle;
	}
	RHI::Fence fence;
	device.CreateFence(&fence, 0);

	uint64_t fenceVal = 0;
	uint32_t currentRtvIndex = 0;
	bool running = true;
	struct FrameResource
	{
		std::uint64_t fence_val = 0;
		RHI::CommandAllocator commandAllocator;
	};
	RHI::GraphicsCommandList commandList;
	FrameResource fResources[3];
	for (auto& s : fResources)
	{
		device.CreateCommandAllocator(RHI::CommandListType::Direct, &s.commandAllocator);
	}
	device.CreateCommandList(RHI::CommandListType::Direct, fResources[1].commandAllocator, &commandList);
	const std::uint32_t framesInFlight = 3;
	std::uint32_t currentFrame = 0;
	RHI::RenderingAttachmentDescStorage color_attachment_desc_mem;
	RHI::RenderingAttachmentDesc CAdesc;
	CAdesc.clearColor = { 0.2f, 1.f, 0.4f, 1.f };
	CAdesc.loadOp = RHI::LoadOp::Clear;
	CAdesc.storeOp = RHI::StoreOp::Store;
	RHI::Area2D render_area;
	render_area.offset = { 0,0 };
	render_area.size = { 1280, 720 };
	RHI::RenderingBeginDesc RBdesc = {};
	RBdesc.numColorAttachments = 1;
	RBdesc.renderingArea = render_area;
	RBdesc.pColorAttachments = &CAdesc;
	RBdesc.pColorAttachmentsMem = &color_attachment_desc_mem;
	RHI::Viewport vp;
	vp.x = 0;  vp.y = 0;
	vp.width = 1280;
	vp.height = 720;
	vp.minDepth = 0.f;
	vp.maxDepth = 1.f;
	std::chrono::time_point clock = std::chrono::high_resolution_clock::now();
	while (running)
	{
		if (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			if (msg.message == WM_QUIT)
				running = false;
		}
		if (!running)
			break;
		std::chrono::time_point clock2 = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(clock2 - clock).count();
		DirectX::XMStoreFloat4x4(&c.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(time * 10))));
		ConstantBuffer.Map(&data);
		memcpy(data, &c, sizeof(c));
		ConstantBuffer.UnMap();
		currentFrame = (currentFrame + 1) % framesInFlight;

		fence.Wait(fResources[currentFrame].fence_val);
		fResources[currentFrame].commandAllocator.Reset();
		commandList.Begin(fResources[currentFrame].commandAllocator);
		DirectX::XMColorEqual(v, DirectX::XMVectorZero());
		CAdesc.ImageView = (handles[currentRtvIndex]);
		
		RHI::TextureMemoryBarrier barr;
		barr.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE;
		barr.oldLayout = RHI::ResourceLayout::UNDEFINED;
		barr.newLayout = RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL;
		barr.subresourceRange = {
			.imageAspect = RHI::Aspect::COLOR_BIT,
			.IndexOrFirstMipLevel = 0,
			.NumMipLevels = 1,
			.FirstArraySlice = 0,
			.NumArraySlices = 1,
		};
		barr.texture  = backBufferImages[currentRtvIndex];
		
		PIPELINE_BARRIER_TEXTURE(commandList, RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::COLOR_ATTACHMENT_OUTPUT_BIT, 1, &barr);
		commandList.BeginRendering(&RBdesc);

		commandList.SetPipelineState(pso);
		commandList.SetRootSignature(rSig);
		commandList.SetDescriptorHeap(dHeap);
		commandList.BindDescriptorSet(rSig, dSet, 0);
		commandList.SetViewports(1, &vp);
		commandList.SetScissorRects(1, &render_area);
		Internal_ID vertexBuffers [] = {VertexBuffer.ID};
		commandList.BindVertexBuffers(0, 1, vertexBuffers);
		commandList.Draw(vertices.size(), 1, 0, 0);

		commandList.EndRendering();
		// Transition the swapchain image back to a presentable layout
		barr.oldLayout = (RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL);
		barr.newLayout = (RHI::ResourceLayout::PRESENT);
		barr.AccessFlagsBefore = (RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE);
		barr.AccessFlagsAfter = (RHI::ResourceAcessFlags::NONE);
		PIPELINE_BARRIER_TEXTURE(commandList, RHI::PipelineStage::COLOR_ATTACHMENT_OUTPUT_BIT, RHI::PipelineStage::BOTTOM_OF_PIPE_BIT, 1, &barr);

		// End command buffer recording
		commandList.End();

		// Submit the command buffer to the queue
		Internal_ID ids[1] = { commandList.ID };
		commandQueue.ExecuteCommandLists(ids, 1);

		fResources[currentFrame].fence_val = ++fenceVal;
		commandQueue.SignalFence(fence, fenceVal);

		swapChain.Present(currentRtvIndex);

		currentRtvIndex = (currentRtvIndex + 1) % 2;
	}
}