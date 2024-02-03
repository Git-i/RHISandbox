#pragma once
#include "Object.h"

namespace RHI
{
	class RHI_API  Texture : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(Texture);
	public:
		void Map(void** data);
		void UnMap();
	};

	enum class TextureType
	{
		Texture1D, Texture2D, Texture3D
	};
	enum class TextureTilingMode
	{
		Linear, Optimal 
	};
	enum class TextureUsage
	{
		None = 0,
		CopyDst = 1 << 0,
		SampledImage = 1 << 1,
		ColorAttachment = 1 << 2,
		DepthStencilAttachment = 1 << 3
	};
	DEFINE_ENUM_FLAG_OPERATORS(TextureUsage);
	struct RHI_API TextureDesc
	{
		TextureType type;
		std::uint32_t width;
		std::uint32_t height;
		std::uint32_t depthOrArraySize;
		Format format;
		std::uint32_t mipLevels;
		std::uint32_t sampleCount;
		TextureTilingMode mode;
		ClearValue* optimizedClearValue = nullptr;
		TextureUsage usage;
	};
	struct RHI_API RenderTargetViewDesc
	{
		bool TextureArray;
		uint32_t arraySlice;
		RHI::Format format;
		uint32_t textureMipSlice;
	};
	struct RHI_API DepthStencilViewDesc
	{
		bool TextureArray;
		uint32_t arraySlice;
		RHI::Format format;
		uint32_t textureMipSlice;
	};
}
