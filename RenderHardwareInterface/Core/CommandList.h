#pragma once
#include "Core.h"
#include "FormatsAndTypes.h"
#include "CommandAllocator.h"
#include "Texture.h"
#include "Barrier.h"
#include "DescriptorHeap.h"
#include "PipelineStateObject.h"
namespace RHI
{
	struct RHI_API RenderingAttachmentDescStorage
	{
	private:
		unsigned char bytes[RENDERING_ATTACHMENT_DESC_DEVICE_SIZE];
	};
	struct RenderingBeginDesc;
	class RHI_API CommandList : public Object
	{
	protected:
		friend class Device;
		Internal_ID m_allocator;
	public:
		DESTROY_FN;
	};
	class RHI_API GraphicsCommandList : public CommandList
	{
	public:
		//CommandAllocator Passed must be the one used to create the command list.
		RESULT Begin(CommandAllocator allocator);
		RESULT PipelineBarrier(PipelineStage syncBefore, PipelineStage syncAfter, std::uint32_t numBufferBarriers, BufferMemoryBarrier* pbufferBarriers, BufferMemoryBarrierStorage* bufferBarriersStorage, std::uint32_t numImageBarriers, TextureMemoryBarrier* pImageBarriers, TextureMemoryBarrierStorage* imageBarriersStorage);
		RESULT SetPipelineState(PipelineStateObject& pso);
		RESULT SetDescriptorHeap(DescriptorHeap heap);
		RESULT BindDescriptorSet(RootSignature rs, DescriptorSet set, std::uint32_t rootParamIndex);
		RESULT SetScissorRects(uint32_t numRects, Area2D* rects);
		RESULT SetViewports(uint32_t numViewports, Viewport* viewports);
		RESULT Draw(uint32_t numVertices, uint32_t numInstances, uint32_t firstVertex, uint32_t firstIndex);
		RESULT BeginRendering(const RenderingBeginDesc* desc);
		RESULT BindVertexBuffers(uint32_t startSlot, uint32_t numBuffers, Internal_ID* buffers);
		RESULT SetRootSignature(RootSignature rs);
		RESULT EndRendering();
		//RESULT SetRenderTargetView(CPU_HANDLE rtv);
		RESULT End();
	};

	struct RHI_API RenderingAttachmentDesc
	{
		CPU_HANDLE ImageView;
		LoadOp loadOp;
		StoreOp storeOp;
		Color clearColor;
	};

	RenderingAttachmentDescStorage RHI_API ConvertToDeviceFormat(const RenderingAttachmentDesc* desc);

	struct RHI_API RenderingBeginDesc
	{
		RenderingAttachmentDescStorage* pColorAttachmentsMem;
		const RenderingAttachmentDesc* pColorAttachments;
		const RenderingAttachmentDesc* pDepthStencilAttachment;
		Area2D renderingArea;
		uint32_t numColorAttachments;
	};

}

#define PIPELINE_BARRIER(commList,syncBefore, syncAfter, numBufferBarriers, pbufferBarriers, numImageBarriers, pImageBarriers)\
{\
RHI::BufferMemoryBarrierStorage bufferStorage[numBufferBarriers];\
RHI::TextureMemoryBarrierStorage textureStorage[numImageBarriers];\
commList.PipelineBarrier(syncBefore, syncAfter, numBufferBarriers, pbufferBarriers, bufferStorage, numImageBarriers, pImageBarriers,textureStorage);\
}
#define PIPELINE_BARRIER_TEXTURE(commList,syncBefore, syncAfter, numImageBarriers, pImageBarriers)\
{\
RHI::TextureMemoryBarrierStorage textureStorage[numImageBarriers];\
commList.PipelineBarrier(syncBefore, syncAfter, 0, NULL, NULL, numImageBarriers, pImageBarriers,textureStorage);\
}
