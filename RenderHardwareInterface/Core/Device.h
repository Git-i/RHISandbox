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
	class RHI_API Device : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(Device);
	public:
		static RESULT Create(PhysicalDevice* PhysicalDevice, CommandQueueDesc const* commandQueueInfos, int numCommandQueues, CommandQueue** commandQueues, Device** device );
		RESULT CreateCommandAllocator(CommandListType type,CommandAllocator** pAllocator);
		template<typename T> RESULT CreateCommandList(CommandListType type,CommandAllocator* allocator,T** pCommandList);
		template<> RESULT CreateCommandList<GraphicsCommandList>(CommandListType type, CommandAllocator* allocator, GraphicsCommandList** ppCommandList);
		RESULT CreateDescriptorHeap(DescriptorHeapDesc* desc, DescriptorHeap** descriptorHeap);
		RESULT CreateDescriptorSets(DescriptorHeap* heap, std::uint32_t numDescriptorSets, DescriptorSetLayout* layouts, DescriptorSet** pSets);
		RESULT UpdateDescriptorSets(std::uint32_t numDescs, DescriptorSetUpdateDesc* desc, DescriptorSet* set);
		RESULT CreateTexture(TextureDesc* desc, Texture** texture, Heap* heap, HeapProperties* props, std::uint64_t offset, ResourceType type);
		RESULT CreateRenderTargetView(Texture* texture, RenderTargetViewDesc* desc, CPU_HANDLE heapHandle);
		RESULT CreateDepthStencilView(Texture* texture, DepthStencilViewDesc* desc, CPU_HANDLE heapHandle);
		RESULT CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject** pPSO);
		RESULT CreateBuffer(BufferDesc* desc, Buffer** buffer, Heap* heap, HeapProperties* props, std::uint64_t offset, ResourceType type);
		RESULT GetBufferMemoryRequirements(BufferDesc* desc, MemoryReqirements* requirements);
		RESULT GetTextureMemoryRequirements(TextureDesc* desc, MemoryReqirements* requirements);
		RESULT CreateRootSignature(RootSignatureDesc* desc, RootSignature** rootSignature, DescriptorSetLayout** pSetLayouts);
		RESULT CreateHeap(HeapDesc* desc, Heap** heap, bool* usedFallback);
		RESULT CreateFence(Fence** fence, std::uint64_t val);
		std::uint32_t GetDescriptorHeapIncrementSize(DescriptorType type);
		RESULT GetSwapChainImage(SwapChain* swapchain, std::uint32_t index, Texture** texture);
		~Device();
	};
}
