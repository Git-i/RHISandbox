#pragma once
#include "Object.h"
namespace RHI
{
	class RHI_API Heap : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(Heap);
	};
	enum class HeapType
	{
		Default,
		Upload,
		Readback,
		Custom
	};
	enum class CPUPageProperty
	{
		WriteCombined = 0,
		WriteCached = 1,
		WriteBack = 1,
		Any = 3,
		NonVisible = 2
	};
	enum class MemoryLevel
	{
		DedicatedRAM,
		SharedRAM,
		Unknown
	};
	struct RHI_API HeapProperties
	{
		HeapType type;
		CPUPageProperty pageProperty;
		MemoryLevel memoryLevel;
		HeapType FallbackType;
		HeapProperties() { memset(this, 0, sizeof(HeapProperties)); };
		HeapProperties(Default_t) {memset(this, 0, sizeof(HeapProperties));};
	};
	struct RHI_API HeapDesc
	{
		std::uint64_t size;
		HeapProperties props;
	};
	struct RHI_API MemoryReqirements
	{
		std::uint64_t size;
		std::uint64_t alignment;
		uint32_t memoryTypeBits;
	};
}
