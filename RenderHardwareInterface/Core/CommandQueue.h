#pragma once
#include "Core.h"
#include "Object.h"
#include "FormatsAndTypes.h"
#include "Fence.h"
namespace RHI
{
	class RHI_API CommandQueue : public Object
	{
	public:
		RESULT SignalFence(Fence fence, std::uint64_t val);
		RESULT ExecuteCommandLists(const Internal_ID* lists, std::uint32_t count);
		DESTROY_FN;
	};
	struct CommandQueueDesc
	{
		CommandListType CommandListType;
		float Priority;
	};
}
