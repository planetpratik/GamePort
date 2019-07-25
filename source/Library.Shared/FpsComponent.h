#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace DX
{
	class FpsComponent final : public DrawableGameComponent
	{
	public:
		FpsComponent(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		FpsComponent(const FpsComponent&) = delete;
		FpsComponent(FpsComponent&&) = default;
		FpsComponent& operator=(const FpsComponent&) = delete;
		FpsComponent& operator=(FpsComponent&&) = default;
		~FpsComponent() = default;

		DirectX::XMFLOAT2& TextPosition();
		int FrameCount() const;	
		int FrameRate() const;

		virtual void CreateDeviceDependentResources() override;
		virtual void Update(const StepTimer& timer) override;
		virtual void Render(const StepTimer& timer) override;

	private:
		std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
		std::unique_ptr<DirectX::SpriteFont> mSpriteFont;
		DirectX::XMFLOAT2 mTextPosition{ 0.0f, 20.0f };

		int mFrameCount{ 0 };
		int mFrameRate{ 0 };
		double mLastTotalSeconds;
	};
}