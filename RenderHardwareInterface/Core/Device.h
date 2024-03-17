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
#include "TextureView.h"
namespace RHI
{
	class RHI_API Device : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(Device);
	public:
		RESULT CreateCommandAllocators(CommandListType type,uint32_t numAllocators,CommandAllocator** pAllocator);
		template<typename T> RESULT CreateCommandList(CommandListType type,CommandAllocator* allocator,T** pCommandList);
		template<> RESULT CreateCommandList<GraphicsCommandList>(CommandListType type, CommandAllocator* allocator, GraphicsCommandList** ppCommandList);
		RESULT CreateDescriptorHeap(DescriptorHeapDesc* desc, DescriptorHeap** descriptorHeap);
		RESULT CreateDynamicDescriptor(DescriptorHeap* heap, DynamicDescriptor** descriptor, DescriptorType type, ShaderStage stage, RHI::Buffer* buffer,uint32_t offset, uint32_t size);
		RESULT CreateTextureView(TextureViewDesc* desc, TextureView** view);
		RESULT CreateDescriptorSets(DescriptorHeap* heap, std::uint32_t numDescriptorSets, DescriptorSetLayout* layouts, DescriptorSet** pSets);
		RESULT UpdateDescriptorSets(std::uint32_t numDescs, DescriptorSetUpdateDesc* desc, DescriptorSet* set);
		RESULT CreateTexture(TextureDesc* desc, Texture** texture, Heap* heap, HeapProperties* props, AutomaticAllocationInfo* automatic_info,std::uint64_t offset, ResourceType type);
		RESULT CreateRenderTargetView(Texture* texture, RenderTargetViewDesc* desc, CPU_HANDLE heapHandle);
		RESULT CreateDepthStencilView(Texture* texture, DepthStencilViewDesc* desc, CPU_HANDLE heapHandle);
		RESULT CreateSampler(SamplerDesc* desc, CPU_HANDLE heapHandle);
		RESULT CreatePipelineStateObject(PipelineStateObjectDesc* desc, PipelineStateObject** pPSO);
		RESULT CreateComputePipeline(ComputePipelineDesc* desc, ComputePipeline** pCP);
		RESULT CreateBuffer(BufferDesc* desc, Buffer** buffer, Heap* heap, HeapProperties* props, AutomaticAllocationInfo* automatic_info, std::uint64_t offset, ResourceType type);
		RESULT GetBufferMemoryRequirements(BufferDesc* desc, MemoryReqirements* requirements);
		RESULT GetTextureMemoryRequirements(TextureDesc* desc, MemoryReqirements* requirements);
		RESULT CreateRootSignature(RootSignatureDesc* desc, RootSignature** rootSignature, DescriptorSetLayout** pSetLayouts);
		RESULT CreateHeap(HeapDesc* desc, Heap** heap, bool* usedFallback);
		RESULT CreateFence(Fence** fence, std::uint64_t val);
		std::uint32_t GetDescriptorHeapIncrementSize(DescriptorType type);
		RESULT GetSwapChainImage(SwapChain* swapchain, std::uint32_t index, Texture** texture);
		~Device();
	};
	const RESULT& vkCompareFunc();
}
extern "C"
{
	RESULT RHI_API RHICreateDevice(RHI::PhysicalDevice* PhysicalDevice, RHI::CommandQueueDesc const* commandQueueInfos, int numCommandQueues, RHI::CommandQueue** commandQueues, Internal_ID instance, RHI::Device** device);
}
