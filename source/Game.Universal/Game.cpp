#include "pch.h"
#include "Game.h"
#include "GameComponent.h"
#include "DrawableGameComponent.h"
#include "OrthographicCamera.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "GamePadComponent.h"
#include "ImGuiComponent.h"
#include "FpsComponent.h"
#include "FieldManager.h"
#include "BallManager.h"
#include "SpriteDemoManager.h"

extern void ExitGame();

using namespace std;
using namespace DX;
using namespace DirectX;

namespace DirectXGame
{
	Game::Game() noexcept(false)
	{
		mDeviceResources = make_shared<DeviceResources>();
		mDeviceResources->RegisterDeviceNotify(this);
	}

	// Initialize the Direct3D resources required to run.
	void Game::Initialize(::IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
	{
		mDeviceResources->SetWindow(window, width, height, rotation);

		mDeviceResources->CreateDeviceResources();

		mDeviceResources->CreateWindowSizeDependentResources();
		CreateWindowSizeDependentResources();

		auto camera = make_shared<OrthographicCamera>(mDeviceResources);
		mComponents.push_back(camera);
		camera->SetPosition(0, 0, 1);

		mKeyboard = make_shared<KeyboardComponent>(mDeviceResources);
		mKeyboard->Keyboard()->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));
		mComponents.push_back(mKeyboard);

		mMouse = make_shared<MouseComponent>(mDeviceResources);
		mMouse->Mouse()->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));
		mComponents.push_back(mMouse);

		mGamePad = make_shared<GamePadComponent>(mDeviceResources);
		mComponents.push_back(mGamePad);

		//auto fieldManager = make_shared<FieldManager>(mDeviceResources, camera);
		//mComponents.push_back(fieldManager);

		//auto ballManager = make_shared<BallManager>(mDeviceResources, camera);		
		//mComponents.push_back(ballManager);

		const int32_t spriteRowCount = 13;
		const int32_t spriteColumnCount = 15;
		auto spriteDemoManager = make_shared<SpriteDemoManager>(mDeviceResources, camera, spriteRowCount, spriteColumnCount);
		//const XMFLOAT2 center((-spriteColumnCount + 1) * SpriteDemoManager::SpriteScale.x, (-spriteRowCount + 1) * SpriteDemoManager::SpriteScale.y);
		const XMFLOAT2 center(0.0f, 0.0f);
		spriteDemoManager->SetPositon(center);
		mComponents.push_back(spriteDemoManager);

		mTimer.SetFixedTimeStep(true);
		mTimer.SetTargetElapsedSeconds(1.0 / 60);

		mFpsComponent = make_shared<FpsComponent>(mDeviceResources);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		auto imGui = make_shared<ImGuiComponent>(mDeviceResources);
		mComponents.push_back(imGui);

		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
			{
				ImGui::Begin("UWP Rendering Demo");
				ImGui::SetNextWindowPos(ImVec2(10, 10));

				{
					stringstream fpsLabel;
					fpsLabel << setprecision(3) << "Frame Rate: " << mFpsComponent->FrameRate() << "  Total Elapsed Time: " << mTimer.GetTotalSeconds() << "  ";
					
					ImGui::Text(fpsLabel.str().c_str());
				}

				ImGui::End();
			});
		imGui->AddRenderBlock(helpTextImGuiRenderBlock);

		InitializeResources();
		//ballManager->SetActiveField(fieldManager->ActiveField());
	}

	void Game::Tick()
	{
		mTimer.Tick([&]()
		{
			Update(mTimer);
		});

		Render();
	}

	void Game::Update(StepTimer const& timer)
	{
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

		for (auto& component : mComponents)
		{
			component->Update(timer);
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) ||
			mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
		{
			ExitGame();
		}

		PIXEndEvent();
	}

	void Game::Render()
	{
		// Don't try to render anything before the first Update.
		if (mTimer.GetFrameCount() == 0)
		{
			return;
		}

		Clear();

		auto context = mDeviceResources->GetD3DDeviceContext();
		PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");

		for (auto& component : mComponents)
		{
			auto drawableComponent = dynamic_pointer_cast<DrawableGameComponent>(component);
			if (drawableComponent != nullptr && drawableComponent->Visible())
			{
				drawableComponent->Render(mTimer);
			}
		}

		PIXEndEvent(context);

		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		mDeviceResources->Present();
		PIXEndEvent();
	}

	// Helper method to clear the back buffers.
	void Game::Clear()
	{
		auto context = mDeviceResources->GetD3DDeviceContext();
		PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

		// Clear the views.
		auto renderTarget = mDeviceResources->GetRenderTargetView();
		auto depthStencil = mDeviceResources->GetDepthStencilView();

		context->ClearRenderTargetView(renderTarget, Colors::Black);
		context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->OMSetRenderTargets(1, &renderTarget, depthStencil);

		// Set the viewport.
		auto viewport = mDeviceResources->GetScreenViewport();
		context->RSSetViewports(1, &viewport);

		PIXEndEvent(context);
	}

#pragma region Message Handlers
	// Message handlers
	void Game::OnActivated()
	{
		// TODO: Game is becoming active window.
	}

	void Game::OnDeactivated()
	{
		// TODO: Game is becoming background window.
	}

	void Game::OnSuspending()
	{
		auto context = mDeviceResources->GetD3DDeviceContext();
		context->ClearState();

		mDeviceResources->Trim();

		// TODO: Game is being power-suspended.
	}

	void Game::OnResuming()
	{
		mTimer.ResetElapsedTime();

		// TODO: Game is being power-resumed.
	}

	void Game::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
	{
		if (!mDeviceResources->WindowSizeChanged(width, height, rotation))
			return;

		CreateWindowSizeDependentResources();

		// TODO: Game window is being resized.
	}

	void Game::ValidateDevice()
	{
		mDeviceResources->ValidateDevice();
	}

	// Properties
	void Game::GetDefaultSize(int& width, int& height) const
	{
		// TODO: Change to desired default window size (note minimum size is 320x200).
		width = 800;
		height = 600;
	}
#pragma endregion

#pragma region Direct3D Resources
	// Allocate all memory resources that change on a window SizeChanged event.
	void Game::CreateWindowSizeDependentResources()
	{
		for (auto& component : mComponents)
		{
			component->CreateWindowSizeDependentResources();
		}
	}

	void Game::OnDeviceLost()
	{
		for (auto& component : mComponents)
		{
			component->ReleaseDeviceDependentResources();
		}
	}

	void Game::OnDeviceRestored()
	{
		InitializeResources();
	}

	void Game::InitializeResources()
	{
		for (auto& component : mComponents)
		{
			component->CreateDeviceDependentResources();
		}

		CreateWindowSizeDependentResources();
	}
#pragma endregion
}