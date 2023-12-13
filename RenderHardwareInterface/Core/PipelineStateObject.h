#pragma once
#include "Core.h"
#include "Object.h"
#include "FormatsAndTypes.h"
#include "vector"
#include "RootSignature.h"
namespace RHI
{
	class RHI_API PipelineStateObject : public Object
	{
		friend class Device;
		friend class GraphicsCommandList;
		std::vector<std::uint32_t> strides;
	};
	enum class InputRate
	{
		Vertex, Instance
	};
	/*
	* Note: Semantic Name and Location have similar function
	* Note: Input slot corresponds to binding in vulkan, used for multiple vertex buffers
	*/
	struct RHI_API InputElementDesc
	{
		std::uint32_t location;
		Format format;
		std::uint32_t inputSlot;
		std::uint32_t alignedByteOffset;
	};
	struct RHI_API InputBindingDesc 
	{
		InputRate inputRate;
		std::uint32_t stride;
	};
	struct RHI_API PipelineStateObjectDesc
	{
		const char* VS = NULL;
		const char* PS = NULL;
		const char* GS = NULL;
		const char* HS = NULL;
		const char* DS = NULL;
		std::uint32_t numInputElements;
		std::uint32_t numInputBindings;
		InputElementDesc* inputElements;
		InputBindingDesc* inputBindings;
		RootSignature rootSig;
		PrimitiveTopology topology;
	};
}
