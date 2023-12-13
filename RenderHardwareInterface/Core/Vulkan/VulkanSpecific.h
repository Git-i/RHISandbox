#pragma once
#include "../FormatsAndTypes.h"
#include "../PhysicalDevice.h"
#include "../Surface.h"
#include "volk.h"
#include <vector>
namespace RHI
{
    enum QueueFamilyIndicesFlags
    {

        HasGraphics = 1 << 1,
        HasCompute = 1 << 2,
        HasPresent = 1 << 3,
        HasCopy = 1 << 4
    };
    DEFINE_ENUM_FLAG_OPERATORS(QueueFamilyIndicesFlags);
    struct QueueFamilyIndices
    {
        std::uint32_t graphicsIndex;
        std::uint32_t computeIndex;
        std::uint32_t copyIndex;
        std::uint32_t presentIndex;
        QueueFamilyIndicesFlags flags;
    };

    QueueFamilyIndices findQueueFamilyIndices(RHI::PhysicalDevice device, RHI::Surface surface = RHI::Surface());


    VkFormat FormatConv(RHI::Format format);
    VkPrimitiveTopology vkPrimitiveTopology(PrimitiveTopology topology);
    extern PFN_vkSetPrivateDataEXT SetPrivateData;
    extern PFN_vkGetPrivateDataEXT GetPrivateData;
}
