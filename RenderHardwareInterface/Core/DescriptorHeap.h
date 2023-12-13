#pragma once
#include "Object.h"
#include "Buffer.h"
#include "FormatsAndTypes.h"
namespace RHI
{
	class RHI_API DescriptorHeap : public Object
	{
	public:
		CPU_HANDLE GetCpuHandle();
	};
	class RHI_API DescriptorSetLayout : public Object
	{
		friend class Device;
		DescriptorHeapType type;
		std::uint32_t numDescriptors;
		std::uint32_t binding;
	};
	class RHI_API DescriptorSet : public Object
	{
		friend class Device;
		friend class GraphicsCommandList;
		CPU_HANDLE start;
		std::uint64_t gpu_handle;
	};

	struct DescriptorBufferInfo
	{
		Buffer buffer;
		std::uint32_t offset;
		std::uint32_t range;
	};
	struct DescriptorTextureInfo
	{
	};
	struct RHI_API DescriptorSetUpdateDesc
	{
		std::uint32_t binding;
		std::uint32_t arrayIndex;
		std::uint32_t numDescriptors;
		DescriptorHeapType type;
		union
		{
			DescriptorBufferInfo* bufferInfos;
			DescriptorTextureInfo* textureInfos;
		};
	};
	struct RHI_API PoolSize
	{
		DescriptorHeapType type;
		std::uint32_t numDescriptors;
	};
	struct RHI_API DescriptorHeapDesc
	{
		std::uint32_t maxDescriptorSets;
		std::uint32_t numPoolSizes;
		PoolSize* poolSizes;
	};
}
