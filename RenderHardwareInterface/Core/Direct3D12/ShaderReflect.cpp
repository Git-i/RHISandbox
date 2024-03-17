#include "pch.h"
#include "D3D12Specific.h"
namespace RHI
{
	//TODO
	RESULT ShaderReflection::CreateFromFile(const char* filename, ShaderReflection** pReflection)
	{
		D3D12ShaderReflection* sr = new D3D12ShaderReflection;
		return 0;
	}
	uint32_t ShaderReflection::GetNumDescriptorSets()
	{
		return 0;
	}
	void ShaderReflection::GetAllDescriptorSets(SRDescriptorSet* set)
	{
		
	}
	void ShaderReflection::GetDescriptorSet(uint32_t set_index, SRDescriptorSet* set)
	{
	}
	void ShaderReflection::GetDescriptorSetBindings(SRDescriptorSet* set, SRDescriptorBinding* bindings)
	{
		
	}
	RESULT ShaderReflection::CreateFromMemory(const char* buffer, ShaderReflection** reflection)
	{
		return RESULT();
	}
}
