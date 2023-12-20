#pragma once
#include "Object.h"
#include "Buffer.h"
#include "FormatsAndTypes.h"
#include "Texture.h"
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
		std::uint32_t numBindings;
		std::uint32_t numDescriptors;
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
	enum class ShaderResourceViewDimension
	{
		Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D, Texture3DArray
	};
	struct DescriptorTextureInfo
	{
		ShaderResourceViewDimension dimension;
		Texture texture;
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
