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
#include "StateManager.h"

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

		mCamera = make_shared<OrthographicCamera>(mDeviceResources);
		mComponents.push_back(mCamera);
		mCamera->SetPosition(0, 0, 1);

		mKeyboard = make_shared<KeyboardComponent>(mDeviceResources);
		mKeyboard->Keyboard()->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));
		mComponents.push_back(mKeyboard);

		mMouse = make_shared<MouseComponent>(mDeviceResources);
		mMouse->Mouse()->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));
		mComponents.push_back(mMouse);

		mGamePad = make_shared<GamePadComponent>(mDeviceResources);
		mComponents.push_back(mGamePad);

		auto stateInstance = StateManager::CreateInstance();
		stateInstance->setState(StateManager::GameState::MAIN_MENU);
		InitializeGameObjects();

		mFpsComponent = make_shared<FpsComponent>(mDeviceResources);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		mTimer.SetFixedTimeStep(true);
		mTimer.SetTargetElapsedSeconds(1.0 / 60);

		mFpsComponent = make_shared<FpsComponent>(mDeviceResources);
		mFpsComponent->SetVisible(false);
		mComponents.push_back(mFpsComponent);

		auto imGui = make_shared<ImGuiComponent>(mDeviceResources);
		mComponents.push_back(imGui);

		auto helpTextImGuiRenderBlock = make_shared<ImGuiComponent::RenderBlock>([this]()
			{
				ImGui::Begin("Balloon Fight");
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

		/*auto device = mDeviceResources->GetD3DDevice();
		ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_NONE;
		auto result = device->CreateRasterizerState(&desc, &mRasterizerState);
		result;*/
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

		// Check game state
		auto instance = StateManager::GetInstance();
		auto state = instance->getState();
		auto activePlayers = instance->getActivePlayers();

		// Update Game entities data
		if (state == StateManager::GameState::MAIN_MENU)
		{
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE)
			{
				// If Key not pressed, keep updating current sprite animation
				mMainMenuBalloons->UpdateData(timer, StateManager::ActivePlayers::PLAYER_ONE);

			}
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
			{
				mMainMenuBalloons->UpdateData(timer, StateManager::ActivePlayers::PLAYER_ONE_AND_TWO);
			}
		}

		if (state == StateManager::GameState::GAME_STARTED)
		{
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE)
			{
			}
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
			{
			}

		}

		// As we have updated data, now actually apply those updates to entity

		for (auto& component : mComponents)
		{
			component->Update(timer);
		}

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) ||
			mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
		{
			ExitGame();
		}

		// Check inputs

		if (state == StateManager::GameState::MAIN_MENU)
		{
			if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::Enter))
			{
				auto it = std::find(mComponents.begin(), mComponents.end(), mMainMenuScreen);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mMainMenuBalloons);
				mComponents.erase(it);
				mComponents.push_back(mPlayerOne);
				mComponents.push_back(mLevelScreen);
				mPlayerOne->CreateDeviceDependentResources();
				mPlayerOne->CreateWindowSizeDependentResources();
				mLevelScreen->CreateDeviceDependentResources();
				mLevelScreen->CreateWindowSizeDependentResources();
				instance->setState(StateManager::GameState::GAME_STARTED);
			}
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

		//context->RSSetState(mRasterizerState);

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
		width = 1200;
		height = 800;
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

	void Game::InitializeGameObjects()
	{
		mMainMenuBalloons = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS);
		mComponents.push_back(mMainMenuBalloons);
		mMainMenuScreen = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::MAIN_MENU_SCREEN);
		mComponents.push_back(mMainMenuScreen);
		auto instance = StateManager::GetInstance();
		instance->setActivePlayers(StateManager::ActivePlayers::PLAYER_ONE);

		mPlayerOne = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::PLAYER_ONE);
		mLevelScreen = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::LEVEL_SCREEN);
	}
#pragma endregion
}