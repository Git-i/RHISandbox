#include "pch.h"
#include "VulkanSpecific.h"
#include "volk.h"
namespace RHI
{
    VkFormat FormatConv(RHI::Format format)
    {
        using namespace RHI;
        switch (format)
        {
            case(Format::UNKNOWN): return VK_FORMAT_UNDEFINED;
            case(Format::R8G8B8A8_UNORM): return VK_FORMAT_R8G8B8A8_UNORM;
            case(Format::B8G8R8A8_UNORM): return VK_FORMAT_B8G8R8A8_UNORM;
            case(Format::R32G32B32A32_FLOAT): return VK_FORMAT_R32G32B32A32_SFLOAT;
            case(Format::R16G16B16A16_FLOAT): return VK_FORMAT_R16G16B16A16_SFLOAT;
            case(Format::R32G32B32_FLOAT): return VK_FORMAT_R32G32B32_SFLOAT;
            case(Format::D32_FLOAT): return VK_FORMAT_D32_SFLOAT;
            case(Format::D24_UNORM_S8_UINT):return VK_FORMAT_D24_UNORM_S8_UINT;
            case(Format::D16_UNORM): return VK_FORMAT_D16_UNORM;
            case(Format::D32_FLOAT_S8X24_UINT): return VK_FORMAT_D32_SFLOAT_S8_UINT;
            case(Format::R32G32_FLOAT): return VK_FORMAT_R32G32_SFLOAT;
            case(Format::R16G16_FLOAT): return VK_FORMAT_R16G16_SFLOAT;
            case(Format::R32_FLOAT): return VK_FORMAT_R32_SFLOAT;
        default: return VK_FORMAT_UNDEFINED;
        }
    }
    VkPrimitiveTopology vkPrimitiveTopology(PrimitiveTopology topology)
    {
        switch (topology)
        {
        case RHI::PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case RHI::PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case RHI::PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case RHI::PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        case RHI::PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        default:
            break;
        }
    }
    std::pair<QueueFamilyIndices,std::vector<uint32_t>> findQueueFamilyIndices(RHI::PhysicalDevice* device, RHI::Surface surface)
    {
        QueueFamilyIndices indices = {};
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties((VkPhysicalDevice)device->ID, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties((VkPhysicalDevice)device->ID, &queueFamilyCount, queueFamilies.data());
        std::vector<uint32_t> count(queueFamilyCount);
        for (int i = 0; i < queueFamilyCount; i++)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsIndex = i;
                count[i] = queueFamilies[i].queueCount;
                indices.flags |= HasGraphics;
            }
            if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                indices.computeIndex = i;
                count[i] = queueFamilies[i].queueCount;
                indices.flags |= HasCompute;
            }
            if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices.copyIndex = i;
                count[i] = queueFamilies[i].queueCount;
                indices.flags |= HasCopy;
            }
            if (surface.ID)
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR((VkPhysicalDevice)device->ID, i, (VkSurfaceKHR)surface.ID, &presentSupport);

                if (presentSupport && !(indices.flags & HasPresent)) {
                    indices.presentIndex = i;
                    indices.flags |= HasPresent;
                }
            }
        }
        return { indices,count };
    }
}