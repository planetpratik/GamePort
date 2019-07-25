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
}

namespace DirectXGame
{
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

		std::shared_ptr<DX::DeviceResources> mDeviceResources;
		DX::StepTimer mTimer;
		std::vector<std::shared_ptr<DX::GameComponent>> mComponents;
		std::shared_ptr<DX::KeyboardComponent> mKeyboard;
		std::shared_ptr<DX::MouseComponent> mMouse;
		std::shared_ptr<DX::GamePadComponent> mGamePad;
		std::shared_ptr<DX::FpsComponent> mFpsComponent;
	};
}