#pragma once
#include "Object.h"

namespace RHI
{
	class RHI_API RootSignature : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(RootSignature);
	};
	enum class ShaderStage
	{
		None = 0,
		Vertex = 1,
		Hull = 2,
		Domain = 4,
		Geometry = 8,
		Pixel = 16,
		Compute = 32,
		AllGraphics = 31,
	};
	DEFINE_ENUM_FLAG_OPERATORS(ShaderStage);
	struct DescriptorRange
	{
		std::uint32_t numDescriptors;
		std::uint32_t BaseShaderRegister;
		ShaderStage stage;
		DescriptorType type;
	};
	struct DescriptorTable
	{
		std::uint32_t numDescriptorRanges;
		std::uint32_t setIndex;
		DescriptorRange* ranges;
	};
	struct DynamicDescriptorDesc
	{
		uint32_t setIndex;
		DescriptorType type;
		ShaderStage stage;
	};
	struct PushConstant
	{
		std::uint32_t numConstants;
	};
	enum class RootParameterType
	{
		DescriptorTable, PushConstant, DynamicDescriptor
	};
	struct RHI_API RootParameterDesc
	{
		RootParameterType type;
		union {
			DescriptorTable descriptorTable;
			PushConstant pushConstant;
			DynamicDescriptorDesc dynamicDescriptor;
		};
	};
	struct RHI_API RootSignatureDesc
	{
		std::uint32_t numRootParameters;
		RootParameterDesc* rootParameters;
	};
}
