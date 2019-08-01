#pragma once

#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "MoodySprite.h"
#include "Sprite.h"
#include "StateManager.h"
#include <vector>
#include <random>
#include <tuple>

namespace DX
{
	class KeyboardComponent;
}

namespace DirectXGame
{
	class SpriteDemoManager final : public DX::DrawableGameComponent
	{
	public:
		SpriteDemoManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera, Sprite::SpriteTypeEnum type = Sprite::SpriteTypeEnum::UNDEFINED);

		const DirectX::XMFLOAT2& Position() const;
		void SetPositon(const DirectX::XMFLOAT2& position);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		DirectX::XMFLOAT2 SpriteScale{ DirectX::XMFLOAT2(2.0f, 4.0f) };
		DirectX::XMFLOAT2 mPosition{ 0.0f, 0.0f };
		DirectX::XMFLOAT2 UVScalingFactor{ DirectX::XMFLOAT2(1.0f / 4,1.0f) };
		std::wstring SpriteSheetName;
		double AnimationUpdateDelay{ 0.2 }; // Delay between Animation changes, in seconds
		double DataUpdateDelay{ 0.5 }; // Delay between Animation changes, in seconds
		double Force{ 2.0 };
		double JumpForce{ 200.0 };

	private:
		struct VSCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
			DirectX::XMFLOAT4X4 TextureTransform;

			VSCBufferPerObject() :
				WorldViewProjection(DX::MatrixHelper::Identity), TextureTransform(DX::MatrixHelper::Identity)
			{ };

			VSCBufferPerObject(const DirectX::XMFLOAT4X4& wvp, const DirectX::XMFLOAT4X4& textureTransform) :
				WorldViewProjection(wvp), TextureTransform(textureTransform)
			{ }
		};

		void InitializeVertices();

		inline static const std::uint32_t SpriteCount{ 8 }; // Sprites are arranged horizontally within the sprite sheet
		inline static const std::uint32_t MoodCount{ 4 }; // Moods are arranged vertically within the sprite sheet
		//inline static const DirectX::XMFLOAT2 UVScalingFactor{ DirectX::XMFLOAT2(/*1.0f / SpriteCount, 1.0f / MoodCount*//*0.25,2*/1.0f/4,1.0f) };
		inline static const double MoodUpdateDelay{ 0.5 }; // Delay between mood changes, in seconds

		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		winrt::com_ptr<ID3D11Buffer> mVSCBufferPerObject;
		winrt::com_ptr<ID3D11ShaderResourceView> mSpriteSheet;
		winrt::com_ptr<ID3D11SamplerState> mTextureSampler;
		winrt::com_ptr<ID3D11BlendState> mAlphaBlending;

		VSCBufferPerObject mVSCBufferPerObjectData;
		std::uint32_t mIndexCount{ 0 };
		std::uint32_t mSpriteRowCount;
		std::uint32_t mSpriteColumnCount;
		double mLastMoodUpdateTime;
		std::random_device mRandomDevice;
		std::default_random_engine mRandomGenerator;
		std::uniform_int_distribution<uint32_t> mSpriteDistribution;
		std::uniform_int_distribution<uint32_t> mSpriteCountDistribution;
		std::uniform_int_distribution<uint32_t> mMoodDistribution;

		// Balloon Fight Related Variables

	public:
		struct RowColumnLookupInfo
		{
			uint32_t SpriteRowCount;
			uint32_t SpriteColumnCount;
			DirectX::XMFLOAT2 Position;
			DirectX::XMFLOAT2 SpriteScale;
			DirectX::XMFLOAT2 UVScalingFactor;
			std::wstring SpriteSheetName;
		};

		enum class SpriteInitialPositions
		{
			UNDEFINED,
			MENU_PLAYER_ONE_BALLOON,
			MENU_PLAYER_TWO_BALLOON,
			PLAYER_ONE,
			PLAYER_TWO
		};

		enum class PlayerMoveDirection
		{
			LEFT,
			RIGHT
		};

		enum class PlayerState
		{
			STANDING,
			FLYING,
			FALLING,
			DEAD
		};

		DirectX::XMFLOAT2 PlayerPos = { 0.0f, 0.0f };
		float PlayerPosX = 0.0f;
		float PlayerPosY = 0.0f;
		inline static const int MAIN_MENU_BACKGROUND_IMAGE_INDEX = 0;
		static const std::unordered_map<Sprite::SpriteTypeEnum, RowColumnLookupInfo> mSpriteRowColumnLookupValuesByType;
		static const std::unordered_map<SpriteInitialPositions, DX::Transform2D> mSpriteInitialPositionsLookup;

		void SetPlayerXMovement(Sprite::SpriteTypeEnum player, float movement);
		void SetPlayerYMovement(Sprite::SpriteTypeEnum player, float movement);
	private:
		winrt::com_ptr<ID3D11ShaderResourceView> mSpriteSheetMainMenu;
		winrt::com_ptr<ID3D11ShaderResourceView> mSpriteSheetMainMenuBalloons;
		bool mIsGameStarted = false;
		std::vector<winrt::com_ptr<ID3D11Texture2D>> mBackgroundImageSprites = {nullptr};
		std::vector<std::shared_ptr<Sprite>> mMenuBalloons;
		std::vector<std::shared_ptr<Sprite>> mSprites;
		Sprite::SpriteTypeEnum mType;
		SpriteInitialPositions mSpriteInitialPositions;
		PlayerMoveDirection mPlayerMoveDirection;
		PlayerMoveDirection mPreviousMoveDirection;
		PlayerState mPlayerState;
		bool isPlayerAlreadyFlipped = false;
		double mLastAnimationUpdateTime;
		double mLastDataUpdateTime;
		uint32_t mCurrentSpriteIndex;
		bool isJumpForceAllowed = true;
		float mJumpAmount = 0;

		DirectX::XMFLOAT2 mPlayerMovement = {0.0f, 0.0f};
		
		void DrawSprite(Sprite& sprite);
		void InitializeSprites(Sprite::SpriteTypeEnum type, std::vector<std::shared_ptr<Sprite>>& mSprites);
		void CalculateCurrentPosition();
	};
}