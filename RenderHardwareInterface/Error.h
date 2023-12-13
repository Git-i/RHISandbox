#pragma once
#if defined(RHI_DIRECT3D12)
#define GFX_CHECK(x) if(x == 0); else {std::cout << x << std::endl;}
#elif defined(RHI_VULKAN)
#define GFX_CHECK(x) if(x == 0); else {std::cout << x << std::endl;}
#endif

#ifdef _DEBUG
#define GFX_ASSERT(cond) if(cond == true); else {__debugbreak();}
#else
#define GFX_ASSERT(cond)
#endif

