#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

namespace DX
{
	class GameComponent;
	class MouseComponent;
	class KeyboardComponent;
	class GamePadComponent;
	class FpsComponent;
	class OrthographicCamera;
}

namespace DirectXGame
{
	class SpriteDemoManager;

	class Game : public DX::IDeviceNotify
	{
	public:
		Game() noexcept(false);

		// Initialization and management
		void Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);

		// Basic game loop
		void Tick();

		// IDeviceNotify
		virtual void OnDeviceLost() override;
		virtual void OnDeviceRestored() override;

		// Messages
		void OnActivated();
		void OnDeactivated();
		void OnSuspending();
		void OnResuming();
		void OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
		void ValidateDevice();

		// Properties
		void GetDefaultSize(int& width, int& height) const;

	private:
		void Update(DX::StepTimer const& timer);
		void Render();
		void Clear();
		void CreateWindowSizeDependentResources();
		void InitializeResources();
		void InitializeGameObjects();

		std::shared_ptr<DX::DeviceResources> mDeviceResources;
		DX::StepTimer mTimer;
		std::vector<std::shared_ptr<DX::GameComponent>> mComponents;
		std::shared_ptr<DX::KeyboardComponent> mKeyboard;
		std::shared_ptr<DX::MouseComponent> mMouse;
		std::shared_ptr<DX::GamePadComponent> mGamePad;
		std::shared_ptr<DX::FpsComponent> mFpsComponent;
		std::shared_ptr<DX::OrthographicCamera> mCamera;

		std::shared_ptr<SpriteDemoManager> mMainMenuScreen;
		std::shared_ptr<SpriteDemoManager> mMainMenuBalloons;
		std::shared_ptr<SpriteDemoManager> mLevelScreen;
		std::shared_ptr<SpriteDemoManager> mPlayerOne;
		std::shared_ptr<SpriteDemoManager> mPlayerTwo;
		std::shared_ptr<SpriteDemoManager> mEnemyOne;
		std::shared_ptr<SpriteDemoManager> mEnemyTwo;
		std::shared_ptr<SpriteDemoManager> mEnemyThree;

		//std::shared_ptr<ID3D11RasterizerState*> mRasterizerState;
		winrt::com_ptr<ID3D11DepthStencilState> m_DepthStencilState;

		ID3D11RasterizerState* mRasterizerState;
		D3D11_RASTERIZER_DESC desc = {};

	};
}