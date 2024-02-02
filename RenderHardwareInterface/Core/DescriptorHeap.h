#pragma once
#include "Buffer.h"
#include "FormatsAndTypes.h"
#include "Object.h"
#include "TextureView.h"
namespace RHI
{
	class RHI_API DescriptorHeap : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(DescriptorHeap);
	public:
		CPU_HANDLE GetCpuHandle();
	};
	class RHI_API DescriptorSetLayout : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(DescriptorSetLayout);
		
	};
	class RHI_API DescriptorSet : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(DescriptorSet);
	};
	class RHI_API DynamicDescriptor : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(DynamicDescriptor);
	};
	struct DescriptorBufferInfo
	{
		Buffer* buffer;
		std::uint32_t offset;
		std::uint32_t range;
	};
	enum class ShaderResourceViewDimension
	{
		Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D, Texture3DArray
	};
	struct DescriptorTextureInfo
	{
		ShaderResourceViewDimension dimension;
		TextureView* texture;
	};
	struct RHI_API DescriptorSetUpdateDesc
	{
		std::uint32_t binding;
		std::uint32_t arrayIndex;
		std::uint32_t numDescriptors;
		DescriptorType type;
		union
		{
			DescriptorBufferInfo* bufferInfos;
			DescriptorTextureInfo* textureInfos;
		};
	};
	struct RHI_API PoolSize
	{
		DescriptorType type;
		std::uint32_t numDescriptors;
	};
	struct RHI_API DescriptorHeapDesc
	{
		std::uint32_t maxDescriptorSets;
		std::uint32_t numPoolSizes;
		PoolSize* poolSizes;
	};
}
