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
	protected:
		DECL_CLASS_CONSTRUCTORS(PipelineStateObject);
		
	};
	//The Naming Conventions is a bit off, because Compute is kind of an afterthought (so is raytracing)
	class RHI_API ComputePipeline : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(ComputePipeline);
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
	enum class SampleMode
	{
		x2 =1,x4=2,x8=3
	};
	enum class BlendFac
	{
		Zero = 0,
		One = 1,
		SrcAlpha = 2,
		InvSrcAlpha = 3,
		DestAlpha = 4,
		InvDescAlpha = 5,
		BlendFactor = 6,
		InvBlendFactor = 7,
		SrcColor = 8,
		InvSrcColor = 9,
		DestColor = 10,
		InvDestColor = 11,
	};
	enum class BlendOp
	{
		Add = 0,
		Subtract = 1,
		RevSubtract = 2,
		Min = 3,
		Max = 4
	};
	enum class LogicOp
	{
		CLEAR         = 0,
		SET           = 1,
		COPY          = 2,
		COPY_INVERTED = 3,
		NOOP          = 4,
		INVERT        = 5,
		AND           = 6,
		NAND          = 7,
		OR            = 8,
		NOR           = 9,
		XOR           = 10,
		EQUIV         = 11,
		AND_REVERSE   = 12,
		AND_INVERTED  = 13,
		OR_REVERSE    = 14,
		OR_INVERTED   = 15
	};
	enum class StencilOp
	{
		Keep = 0,
		Zero = 1,
		Replace = 2,
		IncrSat = 3,
		DecrSat = 4,
		Invert = 5,
		Incr = 6,
		Decr = 7
	};
	struct BlendRenderTargetDesc
	{
		bool blendEnable;
		bool logicOpEnable;
		BlendFac srcColorBlend;
		BlendFac dstColorBlend;
		BlendFac srcAlphaBlend;
		BlendFac dstAlphaBlend;
		BlendOp ColorBlendOp;
		BlendOp AlphaBlendOp;
		LogicOp LogicOp;
		uint8_t writeMask;
	};
	enum class DepthWriteMask
	{
		None = 0, All
	};
	
	struct DepthStencilOp
	{
		StencilOp failOp;
		StencilOp passOp;
		StencilOp DepthfailOp;
		ComparisonFunc Stencilfunc;
	};
	struct DepthStencilMode
	{
		bool DepthEnable;
		RHI::DepthWriteMask DepthWriteMask;
		RHI::ComparisonFunc DepthFunc;
		bool StencilEnable;
		uint8_t StencilReadMask;
		uint8_t StencilWriteMask;
		RHI::DepthStencilOp FrontFace;
		RHI::DepthStencilOp BackFace;
	};
	struct BlendMode
	{
		bool BlendAlphaToCoverage;
		bool IndependentBlend;
		BlendRenderTargetDesc blendDescs[8];
	};
	enum class FillMode
	{
		Solid = 0, Wireframe = 1
	};
	enum class CullMode
	{
		None=0, Front=1, Back=2
	};
	struct RasterizerMode
	{
		FillMode fillMode;
		CullMode cullMode;//front is always ccw
		PrimitiveTopology topology;
		int depthBias;
		float depthBiasClamp;
		float slopeScaledDepthBias;
		bool depthClipEnable;
		bool multisampleEnable;
		bool AntialiasedLineEnable;
		bool conservativeRaster;
	};
	enum ShaderMode
	{
		Memory, File
	};
	struct RHI_API PipelineStateObjectDesc
	{
		ShaderCode VS = {};
		ShaderCode PS = {};
		ShaderCode GS = {};
		ShaderCode HS = {};
		ShaderCode DS = {};
		ShaderMode shaderMode;
		BlendMode blendMode;
		RasterizerMode rasterizerMode;
		std::uint32_t numInputElements;
		std::uint32_t numInputBindings;
		std::uint32_t numRenderTargets;
		InputElementDesc* inputElements;
		InputBindingDesc* inputBindings;
		RootSignature* rootSig;
		

		DepthStencilMode depthStencilMode;

		SampleMode sampleCount;
		Format RTVFormats[8];
		Format DSVFormat;
	};
	struct ComputePipelineDesc
	{
		ShaderCode CS;
		ShaderMode mode;
		RootSignature* rootSig;
	};
}
