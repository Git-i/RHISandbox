#pragma once
#include "Object.h"
#include "Texture.h"
#include "FormatsAndTypes.h"
namespace RHI
{
	enum class TextureViewType
	{
		Texture1D,Texture2D,Texture3D,TextureCube
	};
	struct TextureViewDesc
	{
		TextureViewType type;
		RHI::Format format;
		//TODO 4 component mapping
		RHI::Texture* texture;
		RHI::SubResourceRange range;
	};
	class TextureView : public Object
	{
	protected:
		DECL_CLASS_CONSTRUCTORS(TextureView);
	};
}