#include "pch.h"
#include "Sprite.h"

using namespace DX;
using namespace DirectX;

namespace DirectXGame
{
	Sprite::Sprite(uint32_t spriteIndex, const Transform2D& transform, SpriteTypeEnum type, const XMFLOAT4X4& textureTransform) :
		mSpriteIndex(spriteIndex), mTransform(transform), mSpriteType(type), mTextureTransform(textureTransform)
	{
	}

	uint32_t Sprite::SpriteIndex() const
	{
		return mSpriteIndex;
	}

	void Sprite::SetSpriteIndex(const uint32_t spriteIndex)
	{
		mSpriteIndex = spriteIndex;
	}

	const Transform2D& Sprite::Transform() const
	{
		return mTransform;
	}

	void Sprite::SetTransform(const Transform2D& transform)
	{
		mTransform = transform;
	}

	Sprite::SpriteTypeEnum Sprite::SpriteType() const
	{
		return mSpriteType;
	}

	void Sprite::SetSpriteType(const SpriteTypeEnum type)
	{
		mSpriteType = type;
	}

	const XMFLOAT4X4& Sprite::TextureTransform() const
	{
		return mTextureTransform;
	}

	void Sprite::SetTextureTransform(const XMFLOAT4X4& transform)
	{
		mTextureTransform = transform;
	}
}