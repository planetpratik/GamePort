#pragma once
#include "Transform2D.h"
#include "MatrixHelper.h"

namespace DirectXGame
{
	class Sprite final
	{
	public:
		enum class SpriteTypeEnum
		{
			UNDEFINED,
			MENU_BALLOONS,
			PLAYER_ONE,
			PLAYER_TWO,
			ENEMY_FLYING,
			ENEMY_FALLING,
			ENEMY_PUMPING
		};

		Sprite(std::uint32_t spriteIndex, const DX::Transform2D& transform, SpriteTypeEnum type = SpriteTypeEnum::UNDEFINED, const DirectX::XMFLOAT4X4& textureTransform = DX::MatrixHelper::Identity);

		std::uint32_t SpriteIndex() const;
		void SetSpriteIndex(const std::uint32_t spriteIndex);

		const DX::Transform2D& Transform() const;
		void SetTransform(const DX::Transform2D& transform);

		SpriteTypeEnum SpriteType() const;
		void SetSpriteType(const SpriteTypeEnum type);

		const DirectX::XMFLOAT4X4& TextureTransform() const;
		void SetTextureTransform(const DirectX::XMFLOAT4X4& transform);
	private:
		SpriteTypeEnum mSpriteType;
		DirectX::XMFLOAT4X4 mTextureTransform;
		DX::Transform2D mTransform;
		std::uint32_t mSpriteIndex;
	};
}