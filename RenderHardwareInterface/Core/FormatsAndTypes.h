#pragma once
#include "Core.h"

namespace RHI
{
	enum class ExportOptions
	{
		D3D11TextureNT,
		Win32Handle,
		FD
	};
	enum class Format
	{
		UNKNOWN = 0,
		R32G32B32A32_TYPELESS = 1,
		R32G32B32A32_FLOAT = 2,
		R32G32B32A32_UINT = 3,
		R32G32B32A32_SINT = 4,
		R32G32B32_TYPELESS = 5,
		R32G32B32_FLOAT = 6,
		R32G32B32_UINT = 7,
		R32G32B32_SINT = 8,
		R16G16B16A16_TYPELESS = 9,
		R16G16B16A16_FLOAT = 10,
		R16G16B16A16_UNORM = 11,
		R16G16B16A16_UINT = 12,
		R16G16B16A16_SNORM = 13,
		R16G16B16A16_SINT = 14,
		R32G32_TYPELESS = 15,
		R32G32_FLOAT = 16,
		R32G32_UINT = 17,
		R32G32_SINT = 18,
		R32G8X24_TYPELESS = 19,
		D32_FLOAT_S8X24_UINT = 20,
		R32_FLOAT_X8X24_TYPELESS = 21,
		X32_TYPELESS_G8X24_UINT = 22,
		R10G10B10A2_TYPELESS = 23,
		R10G10B10A2_UNORM = 24,
		R10G10B10A2_UINT = 25,
		R11G11B10_FLOAT = 26,
		R8G8B8A8_TYPELESS = 27,
		R8G8B8A8_UNORM = 28,
		R8G8B8A8_UNORM_SRGB = 29,
		R8G8B8A8_UINT = 30,
		R8G8B8A8_SNORM = 31,
		R8G8B8A8_SINT = 32,
		R16G16_TYPELESS = 33,
		R16G16_FLOAT = 34,
		R16G16_UNORM = 35,
		R16G16_UINT = 36,
		R16G16_SNORM = 37,
		R16G16_SINT = 38,
		R32_TYPELESS = 39,
		D32_FLOAT = 40,
		R32_FLOAT = 41,
		R32_UINT = 42,
		R32_SINT = 43,
		R24G8_TYPELESS = 44,
		D24_UNORM_S8_UINT = 45,
		R24_UNORM_X8_TYPELESS = 46,
		X24_TYPELESS_G8_UINT = 47,
		R8G8_TYPELESS = 48,
		R8G8_UNORM = 49,
		R8G8_UINT = 50,
		R8G8_SNORM = 51,
		R8G8_SINT = 52,
		R16_TYPELESS = 53,
		R16_FLOAT = 54,
		D16_UNORM = 55,
		R16_UNORM = 56,
		R16_UINT = 57,
		R16_SNORM = 58,
		R16_SINT = 59,
		R8_TYPELESS = 60,
		R8_UNORM = 61,
		R8_UINT = 62,
		R8_SNORM = 63,
		R8_SINT = 64,
		A8_UNORM = 65,
		R1_UNORM = 66,
		R9G9B9E5_SHAREDEXP = 67,
		R8G8_B8G8_UNORM = 68,
		G8R8_G8B8_UNORM = 69,
		BC1_TYPELESS = 70,
		BC1_UNORM = 71,
		BC1_UNORM_SRGB = 72,
		BC2_TYPELESS = 73,
		BC2_UNORM = 74,
		BC2_UNORM_SRGB = 75,
		BC3_TYPELESS = 76,
		BC3_UNORM = 77,
		BC3_UNORM_SRGB = 78,
		BC4_TYPELESS = 79,
		BC4_UNORM = 80,
		BC4_SNORM = 81,
		BC5_TYPELESS = 82,
		BC5_UNORM = 83,
		BC5_SNORM = 84,
		B5G6R5_UNORM = 85,
		B5G5R5A1_UNORM = 86,
		B8G8R8A8_UNORM = 87,
		B8G8R8X8_UNORM = 88,
		R10G10B10_XR_BIAS_A2_UNORM = 89,
		B8G8R8A8_TYPELESS = 90,
		B8G8R8A8_UNORM_SRGB = 91,
		B8G8R8X8_TYPELESS = 92,
		B8G8R8X8_UNORM_SRGB = 93,
		BC6H_TYPELESS = 94,
		BC6H_UF16 = 95,
		BC6H_SF16 = 96,
		BC7_TYPELESS = 97,
		BC7_UNORM = 98,
		BC7_UNORM_SRGB = 99,
		AYUV = 100,
		Y410 = 101,
		Y416 = 102,
		NV12 = 103,
		P010 = 104,
		P016 = 105,
		OPAQUE_420 = 106,
		YUY2 = 107,
		Y210 = 108,
		Y216 = 109,
		NV11 = 110,
		AI44 = 111,
		IA44 = 112,
		P8 = 113,
		A8P8 = 114,
		B4G4R4A4_UNORM = 115,
		P208 = 130,
		V208 = 131,
		V408 = 132,
		SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
		SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
	};
	struct Rational
	{
		unsigned int Numerator;
		unsigned int Denominator;
	};
	struct ShaderCode
	{
		char* data = nullptr;
		uint32_t size = 0;
	};
	enum class API
	{
		DX12, Vulkan
	};
	enum class ResourceType
	{
		Placed, Automatic, Commited
	};
	struct Vector2D
	{
		std::int32_t x, y;
	};
	struct UVector2D
	{
		std::uint32_t x, y;
	};
	struct Area2D
	{
		Vector2D offset;
		UVector2D size;
	};
	struct Viewport
	{
		float x, y;
		float width, height;
		float minDepth, maxDepth;
	};
	struct Color
	{
		float r, g, b, a;
	};
	struct DepthStencilValue
	{
		float depth;
		std::uint8_t stecnil;
	};
	struct ClearValue
	{
		Format format;
		union
		{
			Color clearColor;
			DepthStencilValue depthStencilValue;
		};
	};
	enum class CommandListType
	{
		Direct,
		Compute,
		Copy
	};
	enum class LoadOp
	{
		Load, Clear, DontCare
	};
	enum class StoreOp
	{
		Store, DontCare
	};
	enum class DescriptorHeapType
	{
		RTV, DSV, SRV_CBV_UAV, SAMPLER
	};
	enum class DescriptorClass
	{
		CBV, RTV, DSV, SRV, UAV, Sampler
	};
	enum class DescriptorType
	{
		SampledTexture, ConstantBuffer, StructuredBuffer, ConstantBufferDynamic, StructuredBufferDynamic,Sampler,
		RTV,DSV, CSTexture, CSBuffer, CSBufferDynamic
	};
	enum class ComparisonFunc : uint8_t
	{
		Never = 0, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always
	};
	enum class ResourceLayout
	{
		UNDEFINED = 0,
		GENERAL = 1,
		COLOR_ATTACHMENT_OPTIMAL = 2,
		DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
		DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
		SHADER_READ_ONLY_OPTIMAL = 5,
		TRANSFER_SRC_OPTIMAL = 6,
		TRANSFER_DST_OPTIMAL = 7,
		READ_ONLY_OPTIMAL = 1000314000,
		PRESENT = 1000001002,
		VIDEO_DECODE_DST = 1000024000,
		VIDEO_DECODE_SRC = 1000024001,
		FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL = 1000164003,
	};
	enum class Aspect
	{
		COLOR_BIT = 0x00000001,
		DEPTH_BIT = 0x00000002,
		STENCIL_BIT = 0x00000004,
		METADATA_BIT = 0x00000008,
		PLANE_0_BIT = 0x00000010,
		PLANE_1_BIT = 0x00000020,
		PLANE_2_BIT = 0x00000040,
		NONE = 0,
	};
	struct SubResourceRange
	{
		Aspect imageAspect;
		std::uint32_t IndexOrFirstMipLevel;
		std::uint32_t NumMipLevels;
		std::uint32_t FirstArraySlice;
		std::uint32_t NumArraySlices;
	};
	enum class BufferUsage
	{
		None = 0,
		VertexBuffer = 1 << 0,
		ConstantBuffer = 1 << 1,
		IndexBuffer = 1 << 2,
		CopySrc = 1 << 3,
		CopyDst = 1 << 4,
		StructuredBuffer = 1 << 5
	};
	DEFINE_ENUM_FLAG_OPERATORS(BufferUsage);
	enum class PipelineStage
	{
		TOP_OF_PIPE_BIT = 0x00000001,
		DRAW_INDIRECT_BIT = 0x00000002,
		VERTEX_INPUT_BIT = 0x00000004,
		VERTEX_SHADER_BIT = 0x00000008,
		TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
		TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
		GEOMETRY_SHADER_BIT = 0x00000040,
		FRAGMENT_SHADER_BIT = 0x00000080,
		//EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
		//LATE_FRAGMENT_TESTS_BIT = 0x00000200,
		COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
		COMPUTE_SHADER_BIT = 0x00000800,
		TRANSFER_BIT = 0x00001000,
		BOTTOM_OF_PIPE_BIT = 0x00002000,
		//HOST_BIT = 0x00004000,
		ALL_GRAPHICS_BIT = 0x00008000,
		ALL_COMMANDS_BIT = 0x00010000,
		NONE = 0,
		TRANSFORM_FEEDBACK_BIT_EXT = 0x01000000,
		CONDITIONAL_RENDERING_BIT_EXT = 0x00040000,
		ACCELERATION_STRUCTURE_BUILD_BIT_KHR = 0x02000000,
		RAY_TRACING_SHADER_BIT_KHR = 0x00200000,
		FRAGMENT_DENSITY_PROCESS_BIT_EXT = 0x00800000,
		FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x00400000,
		COMMAND_PREPROCESS_BIT_NV = 0x00020000,
		TASK_SHADER_BIT_EXT = 0x00080000,
		MESH_SHADER_BIT_EXT = 0x00100000,
		FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};
	DEFINE_ENUM_FLAG_OPERATORS(PipelineStage);
	enum  class ResourceAcessFlags
	{
		//RHI_ACCESS_INDIRECT_COMMAND_READ_BIT = 0x00000001,
		INDEX_BUFFER_READ = 0x00000002,
		VERTEX_BUFFER_READ = 0x00000004,
		CONSTANT_BUFFER_READ = 0x00000008,
		INPUT_ATTACHMENT_READ = 0x00000010,
		SHADER_READ = 0x00000020,
		SHADER_WRITE = 0x00000040,
		COLOR_ATTACHMENT_READ = 0x00000080,
		COLOR_ATTACHMENT_WRITE = 0x00000100,
		DEPTH_STENCIL_ATTACHMENT_READ = 0x00000200,
		DEPTH_STENCIL_ATTACHMENT_WRITE = 0x00000400,
		TRANSFER_READ = 0x00000800,
		TRANSFER_WRITE = 0x00001000,
		HOST_READ = 0x00002000,
		HOST_WRITE = 0x00004000,
		MEMORY_READ = 0x00008000,
		MEMORY_WRITE = 0x00010000,
		NONE = 0,
		//RHI_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT = 0x02000000,
		//RHI_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT = 0x04000000,
		//RHI_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT = 0x08000000,
		//RHI_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT = 0x00100000,
		//RHI_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT = 0x00080000,
		//RHI_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR = 0x00200000,
		//RHI_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR = 0x00400000,
		//RHI_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT = 0x01000000,
		//RHI_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR = 0x00800000,
		//RHI_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV = 0x00020000,
		//RHI_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV = 0x00040000,
		//RHI_ACCESS_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
	};
	enum class PrimitiveTopology
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};
	enum class AutomaticAllocationCPUAccessMode
	{
		None = 0, Random, Sequential
	};
	struct AutomaticAllocationInfo
	{
		AutomaticAllocationCPUAccessMode access_mode;
	};
	//Structs for tag dispatch
	typedef union { void* ptr; std::uintptr_t val; } CPU_HANDLE;
	typedef struct {} Default_t;
	typedef struct {} Zero_t;
	extern RHI_API Default_t Default;
	extern RHI_API Zero_t Zero;

	
}
