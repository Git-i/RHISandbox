#pragma once
#include "Core.h"
#include "PhysicalDevice.h"
#include "CommandQueue.h"
#include "CommandAllocator.h"
#include "CommandList.h"
#include "DescriptorHeap.h"
#include "Texture.h"
#include "SwapChain.h"
#include "Fence.h"
#include "PipelineStateObject.h"
#include "Buffer.h"
#include "Heap.h"
#include "RootSignature.h"
namespace RHI
{
	class RHI_API Device
	{
	public:
		static RESULT Create(PhysicalDevice PhysicalDevice, CommandQueueDesc const* commandQueueInfos, int numCommandQueues, CommandQueue* commandQueues, Device* device );
		RESULT CreateCommandAllocator(CommandListType type,CommandAllocator* pAllocator);
		RESULT CreateCommandList(CommandListType type,CommandAllocator allocator,CommandList* pCommandList);
		RESULT CreateDescriptorHeap(DescriptorHeapDesc* desc, DescriptorHeap* descriptorHeap);
		RESULT CreateDescriptorSets(DescriptorHeap heap, std::uint32_t numDescriptorSets, DescriptorSetLayout* layouts, DescriptorSet* pSets);
		RESULT UpdateDescriptorSets(std::uint32_t numDescriptorSets, DescriptorSetUpdateDesc* desc, DescriptorSet* set);
		RESULT CreateConstantBufferView();
		RESULT CreateRenderTargetView(Texture texture, RenderTargetViewDesc* desc, CPU_HANDLE heapHandle);
		RESULT CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject* pPSO);
		RESULT CreateBuffer(BufferDesc* desc, Buffer* buffer, Heap* heap, std::uint64_t offset, ResourceType type);
		RESULT GetBufferMemoryRequirements(BufferDesc* desc, MemoryReqirements* requirements);
		RESULT CreateRootSignature(RootSignatureDesc* desc, RootSignature* rootSignature, _Out_ DescriptorSetLayout* pSetLayouts);
		RESULT CreateHeap(HeapDesc* desc, Heap* heap, bool* usedFallback);
		RESULT CreateFence(Fence* fence, std::uint64_t val);
		RESULT WaitForFence(Fence fence, std::uint64_t val);
		std::uint32_t GetDescriptorHeapIncrementSize(DescriptorHeapType type);
		RESULT GetSwapChainImage(SwapChain swapchain, std::uint32_t index, Texture* texture);
		Internal_ID ID;
	};
}
