#pragma once
#include "Object.h"
#include <string>
namespace RHI
{
	struct SRDescriptorBinding
	{
		DescriptorClass resourceClass;
		DescriptorType resourceType;
		uint32_t bindingSlot;
		uint32_t count;
		uint32_t setIndex;
		std::string name;
	};
	struct SRDescriptorSet
	{
		uint32_t setIndex;
		uint32_t bindingCount;
	};
	class RHI_API ShaderReflection : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(ShaderReflection)
	public:
		static RESULT CreateFromFile(const char* filename, ShaderReflection** reflection);
		uint32_t GetNumDescriptorSets();
		void GetAllDescriptorSets(SRDescriptorSet* set);
		void GetDescriptorSet(uint32_t set_index, SRDescriptorSet* set);
		// set is a pointer to a valid SRDescriptorSet, and bindings is pointer to an array of SRDescriptorBinding, with a size of set.bindingCount
		void GetDescriptorSetBindings(SRDescriptorSet* set, SRDescriptorBinding* bindings);;
	};
}
