#pragma once
#include "Object.h"

namespace RHI
{
	class RHI_API RootSignature : public Object
	{

	};
	struct DescriptorRange
	{
		std::uint32_t numDescriptors;
		std::uint32_t BaseShaderRegister;
	};
	struct DescriptorTable
	{
		std::uint32_t numDescriptorRanges;
		DescriptorRange* ranges;
	};
	struct PushConstant
	{
		std::uint32_t numConstants;
	};
	enum class RootParameterType
	{
		DescriptorTable, PushConstant
	};
	struct RHI_API RootParameterDesc
	{
		RootParameterType type;
		union {
			DescriptorTable descriptorTable;
			PushConstant pushConstant;
		};
	};
	struct RHI_API RootSignatureDesc
	{
		std::uint32_t numRootParameters;
		RootParameterDesc* rootParameters;
	};
}