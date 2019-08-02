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
#include "Sprite.h"

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

		InitializeResources();

		// Set Rasterizer Cull mode to None.
		auto device = mDeviceResources->GetD3DDevice();
		ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC));
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.DepthClipEnable = true;
		device->CreateRasterizerState(&desc, mRasterizerState.put());
		auto context = mDeviceResources->GetD3DDeviceContext();
		context->RSSetState(mRasterizerState.get());

		// Create Depth Stencil
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = 0xFF;
		depthStencilDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		device->CreateDepthStencilState(&depthStencilDesc, m_DepthStencilState.put());
		context->OMSetDepthStencilState(m_DepthStencilState.get(), 0);
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
		}

		if (state == StateManager::GameState::GAME_STARTED)
		{
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE)
			{
				CheckCollision(mPlayerOne.get(), mEnemyOne.get());
				CheckCollision(mPlayerOne.get(), mEnemyTwo.get());
				CheckCollision(mPlayerOne.get(), mEnemyThree.get());
			}
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
			{
				CheckCollision(mPlayerOne.get(), mEnemyOne.get());
				CheckCollision(mPlayerOne.get(), mEnemyTwo.get());
				CheckCollision(mPlayerOne.get(), mEnemyThree.get());
				CheckCollision(mPlayerTwo.get(), mEnemyOne.get());
				CheckCollision(mPlayerTwo.get(), mEnemyTwo.get());
				CheckCollision(mPlayerTwo.get(), mEnemyThree.get());
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

		// Check inputs Here

		if (state == StateManager::GameState::MAIN_MENU)
		{
			if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::Enter) || mGamePad->WasButtonDown(DX::GamePadButtons::Start))
			{

				// Remove unused components before adding new components.
				auto it = std::find(mComponents.begin(), mComponents.end(), mMainMenuScreen);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mMainMenuBalloons);
				mComponents.erase(it);

				mMainMenuScreen->ReleaseDeviceDependentResources();
				mMainMenuBalloons->ReleaseDeviceDependentResources();

				if (instance->getActivePlayers() == StateManager::ActivePlayers::PLAYER_ONE)
				{
					mComponents.push_back(mLevelScreen);
					mComponents.push_back(mEnemyOne);
					mComponents.push_back(mEnemyTwo);
					mComponents.push_back(mEnemyThree);
					mComponents.push_back(mPlayerOne);

					//TODO :- Where to dispose Previous resources when switching from level to menu or vice versa ?
					//releaseResources(previousGameState, previousActivePlayers);
					
					mEnemyOne->CreateDeviceDependentResources();
					mEnemyOne->CreateWindowSizeDependentResources();
					mEnemyTwo->CreateDeviceDependentResources();
					mEnemyTwo->CreateWindowSizeDependentResources();
					mEnemyThree->CreateDeviceDependentResources();
					mEnemyThree->CreateWindowSizeDependentResources();
					mPlayerOne->CreateDeviceDependentResources();
					mPlayerOne->CreateWindowSizeDependentResources();
					mLevelScreen->CreateDeviceDependentResources();
					mLevelScreen->CreateWindowSizeDependentResources();
				}
				else
				{
					mComponents.push_back(mLevelScreen);
					mComponents.push_back(mEnemyOne);
					mComponents.push_back(mEnemyTwo);
					mComponents.push_back(mEnemyThree);
					mComponents.push_back(mPlayerOne);
					mComponents.push_back(mPlayerTwo);

					//TODO :- Where to dispose Previous resources when switching from level to menu or vice versa ?
					//releaseResources(previousGameState, previousActivePlayers);
					
					mEnemyOne->CreateDeviceDependentResources();
					mEnemyOne->CreateWindowSizeDependentResources();
					mEnemyTwo->CreateDeviceDependentResources();
					mEnemyTwo->CreateWindowSizeDependentResources();
					mEnemyThree->CreateDeviceDependentResources();
					mEnemyThree->CreateWindowSizeDependentResources();
					mPlayerOne->CreateDeviceDependentResources();
					mPlayerOne->CreateWindowSizeDependentResources();
					mPlayerTwo->CreateDeviceDependentResources();
					mPlayerTwo->CreateWindowSizeDependentResources();
					mLevelScreen->CreateDeviceDependentResources();
					mLevelScreen->CreateWindowSizeDependentResources();
				}
				instance->setState(StateManager::GameState::GAME_STARTED);
			}
			if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::Tab) || mGamePad->WasButtonDown(DX::GamePadButtons::LeftShoulder))
			{
				if (!isPlayerTwoSelected)
				{
					instance->setActivePlayers(StateManager::ActivePlayers::PLAYER_ONE_AND_TWO);
					isPlayerTwoSelected = true;
				}
				else
				{
					instance->setActivePlayers(StateManager::ActivePlayers::PLAYER_ONE);
					isPlayerTwoSelected = false;
				}
			}
		}

		if (state == StateManager::GameState::GAME_STARTED)
		{
			if (instance->getActivePlayers() == StateManager::ActivePlayers::PLAYER_ONE)
			{
				if (mKeyboard->WasKeyDown(DX::Keys::A) || mGamePad->WasButtonDown(DX::GamePadButtons::DPadLeft))
				{
					mPlayerOne->SetPlayerXMovement(Sprite::SpriteTypeEnum::PLAYER_ONE, -2.0f);
				}
				if (mKeyboard->WasKeyDown(DX::Keys::D) || mGamePad->WasButtonDown(DX::GamePadButtons::DPadRight))
				{
					mPlayerOne->SetPlayerXMovement(Sprite::SpriteTypeEnum::PLAYER_ONE, 2.0f);
				}
				if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::J) || mGamePad->WasButtonDown(DX::GamePadButtons::A))
				{
					mPlayerOne->SetPlayerYMovement(Sprite::SpriteTypeEnum::PLAYER_ONE, 2.0f);
				}
				// Currently you can't come back to main menu as game crashes whenever i dispose off components.

				/*if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::Back))
				{
					instance->setState(StateManager::GameState::MAIN_MENU);
					previousGameState = StateManager::GameState::GAME_STARTED;
					previousActivePlayers = StateManager::ActivePlayers::PLAYER_ONE;
					removeComponents(previousGameState, previousActivePlayers);
				}*/
			}

			else
			{
				if (mKeyboard->WasKeyDown(DX::Keys::A) || mGamePad->WasButtonDown(DX::GamePadButtons::DPadLeft))
				{
					mPlayerOne->SetPlayerXMovement(Sprite::SpriteTypeEnum::PLAYER_ONE, -2.0f);
				}
				if (mKeyboard->WasKeyDown(DX::Keys::D) || mGamePad->WasButtonDown(DX::GamePadButtons::DPadRight))
				{
					mPlayerOne->SetPlayerXMovement(Sprite::SpriteTypeEnum::PLAYER_ONE, 2.0f);
				}
				if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::J) || mGamePad->WasButtonDown(DX::GamePadButtons::A))
				{
					mPlayerOne->SetPlayerYMovement(Sprite::SpriteTypeEnum::PLAYER_ONE, 2.0f);
				}
				if (mKeyboard->WasKeyDown(DX::Keys::Left))
				{
					mPlayerTwo->SetPlayerXMovement(Sprite::SpriteTypeEnum::PLAYER_TWO, -2.0f);
				}
				if (mKeyboard->WasKeyDown(DX::Keys::Right))
				{
					mPlayerTwo->SetPlayerXMovement(Sprite::SpriteTypeEnum::PLAYER_TWO, 2.0f);
				}
				if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::PageDown))
				{
					mPlayerTwo->SetPlayerYMovement(Sprite::SpriteTypeEnum::PLAYER_TWO, 2.0f);
				}

				// Currently you can't come back to main menu as game crashes whenever i dispose off components.

				/*if (mKeyboard->WasKeyPressedThisFrame(DX::Keys::Back))
				{
					instance->setState(StateManager::GameState::MAIN_MENU);
					previousGameState = StateManager::GameState::GAME_STARTED;
					previousActivePlayers = StateManager::ActivePlayers::PLAYER_ONE_AND_TWO;
					removeComponents(previousGameState, previousActivePlayers);
				}*/
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

		// Clear the views.
		auto renderTarget = mDeviceResources->GetRenderTargetView();
		auto depthStencil = mDeviceResources->GetDepthStencilView();
		context->ClearRenderTargetView(renderTarget, Colors::Purple);
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
		mMainMenuScreen = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::MAIN_MENU_SCREEN);
		mComponents.push_back(mMainMenuScreen);
		mMainMenuBalloons = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS);
		mComponents.push_back(mMainMenuBalloons);
		auto instance = StateManager::GetInstance();
		instance->setActivePlayers(StateManager::ActivePlayers::PLAYER_ONE);

		mPlayerOne = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::PLAYER_ONE);
		mPlayerTwo = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::PLAYER_TWO);
		mLevelScreen = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::LEVEL_SCREEN);
		mEnemyOne = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::ENEMY_ONE);
		mEnemyTwo = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::ENEMY_TWO);
		mEnemyThree = make_shared<SpriteDemoManager>(mDeviceResources, mCamera, Sprite::SpriteTypeEnum::ENEMY_THREE);
	}

	std::pair<bool, SpriteDemoManager*> Game::CheckCollision(SpriteDemoManager* objectOne, SpriteDemoManager* objectTwo)
	{
		auto objOneXPos = objectOne->GetXPosition();
		auto objTwoXPos = objectTwo->GetXPosition();
		auto objOneXSize = objectOne->GetXSize();
		auto objTwoXSize = objectTwo->GetXSize();
		auto objOneYPos = objectOne->GetYPosition();
		auto objTwoYPos = objectTwo->GetYPosition();
		auto objOneYSize = objectOne->GetYSize();
		auto objTwoYSize = objectTwo->GetYSize();
		// If X axis collides.
		bool collisionX = objOneXPos + objOneXSize/2 >= objTwoXPos &&
			objTwoXPos + objTwoXSize/2 >= objOneXPos;
		// If Y axis collides.
		bool collisionY = objOneYPos + objOneYSize/ 2 >= objTwoYPos &&
			objTwoYPos + objTwoYSize/ 2 >= objOneYPos;
		// Collision only if on both axes
		if (collisionX && collisionY)
		{
			// Okay. It means some part collided. we are only interested in find out which part of body collided. 
			// so we can determine who was on the top of who. 
			
			// YET TO IMPLEMENT 
		}

		// FOR CHECKING ONLY 
		return std::make_pair(true, objectTwo);
	}

	void Game::removeComponents(StateManager::GameState state, StateManager::ActivePlayers activePlayers)
	{
		if (state == StateManager::GameState::GAME_STARTED)
		{
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE)
			{
				auto it = std::find(mComponents.begin(), mComponents.end(), mLevelScreen);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mPlayerOne);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mEnemyOne);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mEnemyTwo);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mEnemyThree);
				mComponents.erase(it);

				mComponents.push_back(mMainMenuScreen);
				mComponents.push_back(mMainMenuBalloons);
			}
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
			{
				auto it = std::find(mComponents.begin(), mComponents.end(), mLevelScreen);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mPlayerOne);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mPlayerTwo);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mEnemyOne);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mEnemyTwo);
				mComponents.erase(it);
				it = std::find(mComponents.begin(), mComponents.end(), mEnemyThree);
				mComponents.erase(it);

				mComponents.push_back(mMainMenuScreen);
				mComponents.push_back(mMainMenuBalloons);
			}
		}
	}

	void Game::releaseResources(StateManager::GameState state, StateManager::ActivePlayers activePlayers)
	{
		if (state == StateManager::GameState::GAME_STARTED)
		{
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE)
			{
				mLevelScreen->ReleaseDeviceDependentResources();
				mPlayerOne->ReleaseDeviceDependentResources();
				mEnemyOne->ReleaseDeviceDependentResources();
				mEnemyTwo->ReleaseDeviceDependentResources();
				mEnemyThree->ReleaseDeviceDependentResources();
			}
			if (activePlayers == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
			{
				mLevelScreen->ReleaseDeviceDependentResources();
				mPlayerOne->ReleaseDeviceDependentResources();
				mPlayerTwo->ReleaseDeviceDependentResources();
				mEnemyOne->ReleaseDeviceDependentResources();
				mEnemyTwo->ReleaseDeviceDependentResources();
				mEnemyThree->ReleaseDeviceDependentResources();
			}
		}
	}

#pragma endregion
}