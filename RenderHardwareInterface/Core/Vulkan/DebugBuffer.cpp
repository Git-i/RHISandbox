#include "../DebugBuffer.h"
#include "VulkanSpecific.h"
#include "VulkanAfterCrash.h"
namespace RHI
{
    uint32_t DebugBuffer::GetValue()
    {
        return *((vDebugBuffer*)this)->data;
    }

}
