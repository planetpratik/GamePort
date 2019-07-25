#include "pch.h"
#include "FpsComponent.h"
#include "StepTimer.h"

using namespace std;
using namespace std::literals;
using namespace DirectX;
using namespace DX;

namespace DX
{
	FpsComponent::FpsComponent(const shared_ptr<DX::DeviceResources>& deviceResources) :
		DrawableGameComponent(deviceResources)		
	{
	}

	XMFLOAT2& FpsComponent::TextPosition()
	{
		return mTextPosition;
	}

	int FpsComponent::FrameCount() const
	{
		return mFrameCount;
	}

	int FpsComponent::FrameRate() const
	{
		return mFrameRate;
	}

	void FpsComponent::CreateDeviceDependentResources()
	{
		mSpriteBatch = make_unique<SpriteBatch>(mDeviceResources->GetD3DDeviceContext());
		mSpriteFont = make_unique<SpriteFont>(mDeviceResources->GetD3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");
	}

	void FpsComponent::Update(const StepTimer& timer)
	{
		if (timer.GetTotalSeconds() - mLastTotalSeconds >= 1.0)
		{
			mLastTotalSeconds = timer.GetTotalSeconds();
			mFrameRate = mFrameCount;
			mFrameCount = 0;
		}

		++mFrameCount;
	}

	void FpsComponent::Render(const StepTimer& gameTime)
	{
		mSpriteBatch->Begin();

		wostringstream fpsLabel;
		fpsLabel << setprecision(4) << L"Frame Rate: " << mFrameRate << L"    Total Elapsed Time: " << gameTime.GetTotalSeconds();
		mSpriteFont->DrawString(mSpriteBatch.get(), fpsLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
	}
}