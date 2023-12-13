#pragma once
#include "Core.h"
#include "../FormatsAndTypes.h"
#include "include\d3d12.h"
#include <vector>
namespace RHI
{
    D3D12_BARRIER_ACCESS D3DAcessMode(ResourceAcessFlags flags);
    D3D12_BARRIER_LAYOUT D3DBarrierLayout(ResourceLayout layout);
    D3D12_BARRIER_SYNC   D3DBarrierSync(PipelineStage stage);
    
}
