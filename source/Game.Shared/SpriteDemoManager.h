#pragma once

#include "DrawableGameComponent.h"
#include "MatrixHelper.h"
#include "MoodySprite.h"
#include <vector>
#include <random>

namespace DirectXGame
{
	class SpriteDemoManager final : public DX::DrawableGameComponent
	{
	public:
		SpriteDemoManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera, std::uint32_t spriteRowCount = 1, std::uint32_t spriteColumCount = 8);

		const DirectX::XMFLOAT2& Position() const;
		void SetPositon(const DirectX::XMFLOAT2& position);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

		inline static const DirectX::XMFLOAT2 SpriteScale{ DirectX::XMFLOAT2(3.0f, 3.0f) };

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

		void DrawSprite(MoodySprite& sprite);
		void InitializeVertices();
		void InitializeSprites();
		void ChangeMood(MoodySprite& sprite);
		MoodySprite::Moods GetRandomMood();

		inline static const std::uint32_t SpriteCount{ 8 }; // Sprites are arranged horizontally within the sprite sheet
		inline static const std::uint32_t MoodCount{ 4 }; // Moods are arranged vertically within the sprite sheet
		inline static const DirectX::XMFLOAT2 UVScalingFactor{ DirectX::XMFLOAT2(1.0f / SpriteCount, 1.0f / MoodCount) };
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
		std::vector<std::shared_ptr<MoodySprite>> mSprites;
		std::uint32_t mIndexCount{ 0 };
		std::uint32_t mSpriteRowCount;
		std::uint32_t mSpriteColumnCount;
		DirectX::XMFLOAT2 mPosition{ 0.0f, 0.0f };
		double mLastMoodUpdateTime;
		std::random_device mRandomDevice;
		std::default_random_engine mRandomGenerator;
		std::uniform_int_distribution<uint32_t> mSpriteDistribution;
		std::uniform_int_distribution<uint32_t> mSpriteCountDistribution;
		std::uniform_int_distribution<uint32_t> mMoodDistribution;

		// Balloon Fight Related Variables

	public:
		inline static const DirectX::XMFLOAT2 BackgroundImageScale{ DirectX::XMFLOAT2(50.0f, 50.0f) };
		inline static const int MAIN_MENU_BACKGROUND_IMAGE_INDEX = 0;

	private:
		winrt::com_ptr<ID3D11ShaderResourceView> mSpriteSheetMainMenu;
		bool mIsGameStarted = false;
		std::vector<winrt::com_ptr<ID3D11Texture2D>> mBackgroundImageSprites = {nullptr};
		void DrawSprite(/*winrt::com_ptr<ID3D11Texture2D>*/ int backgroundImageSprite);
	};
}