#include "pch.h"
#include "SpriteDemoManager.h"
#include "StepTimer.h"
#include "VertexDeclarations.h"
#include "Utility.h"
#include "Camera.h"
#include "KeyboardComponent.h"
#include "Random.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	const std::unordered_map<Sprite::SpriteTypeEnum, SpriteDemoManager::RowColumnLookupInfo> SpriteDemoManager::mSpriteRowColumnLookupValuesByType =
	{
		{ Sprite::SpriteTypeEnum::MAIN_MENU_SCREEN, {1u, 1u, {0.0, 0.0}, {50, 50}, {1.0f, 1.0f}, L"Content\\Textures\\StartScreen.png"}},
		{ Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS, {1u, 4u, {-20.0f, -10.0f}, {2, 4}, {1.0f / 4, 1.0f}, L"Content\\Textures\\StartScreenBalloons.png"}},
		{ Sprite::SpriteTypeEnum::LEVEL_SCREEN, {1u, 1u, {0.0, 0.0}, {50, 50}, {1.0f, 1.0f}, L"Content\\Textures\\Level.png"}},
		{ Sprite::SpriteTypeEnum::PLAYER_ONE, {/*7*/1u, 10u, {-40.0, -35.0}, {2.5, 4}, {1.0f / 9, 1.0f}, L"Content\\Textures\\Player_One.png"}},
		{ Sprite::SpriteTypeEnum::ENEMY_ONE, {1u, 10u, {-15.0, 6.0}, {4, 4}, {1.0f / 9, 1.0f}, L"Content\\Textures\\Enemy.png"}},
		{ Sprite::SpriteTypeEnum::ENEMY_TWO, {1u, 10u, {0.0, 6.0}, {4, 4}, {1.0f / 9, 1.0f}, L"Content\\Textures\\Enemy.png"}},
		{ Sprite::SpriteTypeEnum::ENEMY_THREE, {1u, 10u, {15.0, 6.0}, {4, 4}, {1.0f / 9, 1.0f}, L"Content\\Textures\\Enemy.png"}},
	};

	const std::unordered_map<SpriteDemoManager::SpriteInitialPositions, DX::Transform2D> SpriteDemoManager::mSpriteInitialPositionsLookup =
	{
		{ SpriteDemoManager::SpriteInitialPositions::MENU_PLAYER_ONE_BALLOON, DX::Transform2D(XMFLOAT2(-20.0f, -10.0f), 0.0f, XMFLOAT2(2,4))},
		{ SpriteDemoManager::SpriteInitialPositions::MENU_PLAYER_TWO_BALLOON, DX::Transform2D(XMFLOAT2(-20.0f, -14.0f), 0.0f, XMFLOAT2(2,4))},
		{ SpriteDemoManager::SpriteInitialPositions::PLAYER_ONE, DX::Transform2D(XMFLOAT2(-40.0f, -35.0f), 0.0f, XMFLOAT2(2.5,4))},
	};

	SpriteDemoManager::SpriteDemoManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera, Sprite::SpriteTypeEnum type) :
		DrawableGameComponent(deviceResources, camera), mType(type), mCurrentSpriteIndex(0), mSpriteInitialPositions(SpriteInitialPositions::UNDEFINED)
	{
		SpriteScale = mSpriteRowColumnLookupValuesByType.at(type).SpriteScale;
		UVScalingFactor = mSpriteRowColumnLookupValuesByType.at(type).UVScalingFactor;
		mPosition = mSpriteRowColumnLookupValuesByType.at(type).Position;
		SpriteSheetName = mSpriteRowColumnLookupValuesByType.at(type).SpriteSheetName;
		mPlayerMoveDirection = PlayerMoveDirection::RIGHT;
		/*if (mType == Sprite::SpriteTypeEnum::ENEMY_ONE || mType == Sprite::SpriteTypeEnum::ENEMY_TWO || mType == Sprite::SpriteTypeEnum::ENEMY_THREE)
		{
			mPlayerState = PlayerState::FLYING;
		}*/
		mPlayerState = PlayerState::STANDING;
		PlayerPos = mSpriteRowColumnLookupValuesByType.at(type).Position;
	}

	const XMFLOAT2& SpriteDemoManager::Position() const
	{
		return mPosition;
	}

	void SpriteDemoManager::SetPositon(const XMFLOAT2& position)
	{
		mPosition = position;
	}

	void SpriteDemoManager::CreateDeviceDependentResources()
	{
		auto compiledVertexShader = Utility::LoadBinaryFile(L"Content\\Shaders\\SpriteRendererVS.cso");
		mVertexShader = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateVertexShader(compiledVertexShader.data(), compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");
		mInputLayout = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateInputLayout(VertexPositionTexture::InputElements.data(), narrow_cast<uint32_t>(VertexPositionTexture::InputElements.size()), compiledVertexShader.data(), compiledVertexShader.size(), mInputLayout.put()), "ID3D11Device::CreateInputLayout() failed.");

		const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(VSCBufferPerObject), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");

		auto compiledPixelShader = Utility::LoadBinaryFile(L"Content\\Shaders\\SpriteRendererPS.cso");
		mPixelShader = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreatePixelShader(compiledPixelShader.data(), compiledPixelShader.size(), nullptr, mPixelShader.put()), "ID3D11Device::CreatedPixelShader() failed.");

		const CD3D11_DEFAULT defaultDesc;
		const CD3D11_SAMPLER_DESC samplerStateDesc(defaultDesc);
		mTextureSampler = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateSamplerState(&samplerStateDesc, mTextureSampler.put()));

		CD3D11_BLEND_DESC blendStateDesc(defaultDesc);
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		mAlphaBlending = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBlendState(&blendStateDesc, mAlphaBlending.put()));

		// Load Appropriate SpriteSheet in SRV
		mSpriteSheet = nullptr;
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mDeviceResources->GetD3DDevice(), SpriteSheetName.c_str(), nullptr, mSpriteSheet.put()));
		InitializeVertices();
		InitializeSprites(mType, mSprites);
	}

	void SpriteDemoManager::ReleaseDeviceDependentResources()
	{
		mVertexShader = nullptr;
		mPixelShader = nullptr;
		mInputLayout = nullptr;
		mVertexBuffer = nullptr;
		mIndexBuffer = nullptr;
		mVSCBufferPerObject = nullptr;
		mSpriteSheet = nullptr;
		mTextureSampler = nullptr;

		mSpriteSheetMainMenu = nullptr;
		mSpriteSheetMainMenuBalloons = nullptr;
	}

	void SpriteDemoManager::SetPlayerXMovement(Sprite::SpriteTypeEnum player, float movement)
	{
		if (player == Sprite::SpriteTypeEnum::PLAYER_ONE)
		{
			if (mPlayerState == PlayerState::STANDING || mPlayerState == PlayerState::FLYING || mPlayerState == PlayerState::FALLING)
			{
				// Simply flip Sprite based on current orientation
				if (mPlayerMoveDirection == PlayerMoveDirection::LEFT && movement > 0)
				{
					mPlayerMoveDirection = PlayerMoveDirection::RIGHT;
					//isPlayerAlreadyFlipped = false;
				}
				if (mPlayerMoveDirection == PlayerMoveDirection::RIGHT && movement < 0)
				{
					mPlayerMoveDirection = PlayerMoveDirection::LEFT;
				}
			}
			if (mPlayerState != PlayerState::DEAD)
			{
				// If player is alive then only update position,
				mPlayerMovement.x += movement / 8;

				// If player goes out of screen, make him appear entering from other side.

				if (mPlayerMovement.x > 88)
				{
					mPlayerMovement.x = -8;
				}
				if (mPlayerMovement.x < -8)
				{
					mPlayerMovement.x = 88;
				}
				// Whenever player walks beyond platform, make him fall
				if (PlayerPosY >= 0 && PlayerPosY <= 6)
				{
					if ((PlayerPosX >= 25 || PlayerPosX <= -25) && mPlayerState == PlayerState::STANDING)
					{
						mPlayerState = PlayerState::FALLING;
					}
				}
			}
		}
	}

	void SpriteDemoManager::SetPlayerYMovement(Sprite::SpriteTypeEnum player, float movement)
	{
		if (player == Sprite::SpriteTypeEnum::PLAYER_ONE)
		{
			if (mPlayerState != PlayerState::DEAD)
			{
				// If player is alive, then only update position,
				if (isJumpForceAllowed)
				{
					mJumpAmount += movement * static_cast<float>(JumpForce) / 40;
					mCurrentSpriteIndex = 1;
					mPlayerState = PlayerState::FLYING;
					isJumpForceAllowed = false;
				}

				// Don't let player go out of upper y-bounds.
				if (mPlayerMovement.y > 80)
				{
					mPlayerMovement.y = 79;
				}
				// check if players top touching bottom of middle platform. if yes, then block
				if (PlayerPosY >= -4.5f && PlayerPosY < -1.0f)
				{
					(void)0;
					if (PlayerPosX > -25.0f && PlayerPosX < 25.0f)
					{
						mJumpAmount = 0;
						isJumpForceAllowed = true;
						mPlayerMovement.y -= 1;
						PlayerPosY -= 1;
						mPlayerState = PlayerState::FALLING;
					}
				}
			}
		}
	}

	void SpriteDemoManager::Update(const StepTimer& timer)
	{
		// For every update, do change movement. Note that movement update and animation update are different.
		auto instance = StateManager::GetInstance();
		auto state = instance->getState();
		CalculateCurrentPosition();

		if (state == StateManager::GameState::GAME_STARTED && mPlayerState != PlayerState::DEAD)
		{
			if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE && mPlayerState == PlayerState::FLYING)
			{
				if (mJumpAmount > 0)
				{
					mPlayerMovement.y += 0.25;
					mJumpAmount -= 0.05;
				}
				if (mJumpAmount <= 0)
				{
					mPlayerState = PlayerState::FALLING;
				}
			}
			else if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE && mPlayerState == PlayerState::FALLING)
			{
				// If player is landing on ground, don't let him fall through.
				if (PlayerPosY < -35.0f)
				{
					if ((PlayerPosX > -45.0f && PlayerPosX < -30.0f) || (PlayerPosX > 30.0f && PlayerPosX < 45.0f))
					{
						mJumpAmount = 0;
						isJumpForceAllowed = true;
						mPlayerMovement.y += 1;
						PlayerPosY = -35.0;
						mPlayerState = PlayerState::STANDING;
					}
				}
				if (PlayerPosY <= 5 && PlayerPosY > 0.0)
				{
					if (PlayerPosX > -25.0f && PlayerPosX < 25.0f)
					{
						mJumpAmount = 0;
						isJumpForceAllowed = true;
						mPlayerMovement.y += 1;
						PlayerPosY = 0.0;
						mPlayerState = PlayerState::STANDING;
					}
				}

				// Player Goes below screen but stays there ( not visible )
				if (mPlayerMovement.y < -20)
				{
					mPlayerMovement.y = -19;
				}
				else
				{
					mPlayerMovement.y -= 0.2;
				}
			}
			else if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE && mPlayerState == PlayerState::STANDING)
			{

			}
			if (mType == Sprite::SpriteTypeEnum::ENEMY_ONE)
			{
				if (mPlayerState == PlayerState::FLYING)
				{
					UpdateEnemyMovements();
				}
			}
			if (mType == Sprite::SpriteTypeEnum::ENEMY_TWO)
			{
				if (mPlayerState == PlayerState::FLYING)
				{
					UpdateEnemyMovements();
				}
			}
			if (mType == Sprite::SpriteTypeEnum::ENEMY_THREE)
			{
				if (mPlayerState == PlayerState::FLYING)
				{
					UpdateEnemyMovements();
				}
			}
		}

		// Update Sprites only if sprite animation time has been elapsed. 
		if (timer.GetTotalSeconds() > mLastAnimationUpdateTime + AnimationUpdateDelay)
		{
			mLastAnimationUpdateTime = timer.GetTotalSeconds();
			//auto activePlayer = instance->getActivePlayers();
			// If currently in the Main Menu
			if (state == StateManager::GameState::MAIN_MENU)
			{
				if (mCurrentSpriteIndex < mSprites.size() - 1)
				{
					++mCurrentSpriteIndex;
				}
				else
				{
					mCurrentSpriteIndex = 0;
				}
			}

			// If Player(s) is/are in the game.
			// Do player & Enemy animations here.
			if (state == StateManager::GameState::GAME_STARTED)
			{
				if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE && mPlayerState == PlayerState::FLYING)
				{
					if (mCurrentSpriteIndex < 3)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mCurrentSpriteIndex = 1;
						isJumpForceAllowed = true;
						mJumpAmount = 0;
					}
				}
				if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE && mPlayerState == PlayerState::STANDING)
				{
					mCurrentSpriteIndex = 0;
				}
				if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE && mPlayerState == PlayerState::FALLING)
				{
					mCurrentSpriteIndex = 3;
				}
				if (mType == Sprite::SpriteTypeEnum::ENEMY_ONE && mPlayerState == PlayerState::STANDING)
				{
					if (mCurrentSpriteIndex < 2)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mPlayerState = PlayerState::FLYING;
					}
				}
				if (mType == Sprite::SpriteTypeEnum::ENEMY_TWO && mPlayerState == PlayerState::STANDING)
				{
					if (mCurrentSpriteIndex < 2)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mPlayerState = PlayerState::FLYING;
					}
				}
				if (mType == Sprite::SpriteTypeEnum::ENEMY_THREE && mPlayerState == PlayerState::STANDING)
				{
					if (mCurrentSpriteIndex < 2)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mPlayerState = PlayerState::FLYING;
					}
				}
				if (mType == Sprite::SpriteTypeEnum::ENEMY_ONE && mPlayerState == PlayerState::FLYING)
				{
					if (mCurrentSpriteIndex < 5)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mCurrentSpriteIndex = 3;
					}
				}
				if (mType == Sprite::SpriteTypeEnum::ENEMY_TWO && mPlayerState == PlayerState::FLYING)
				{
					if (mCurrentSpriteIndex < 5)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mCurrentSpriteIndex = 3;
					}
				}
				if (mType == Sprite::SpriteTypeEnum::ENEMY_THREE && mPlayerState == PlayerState::FLYING)
				{
					if (mCurrentSpriteIndex < 5)
					{
						++mCurrentSpriteIndex;
					}
					else
					{
						mCurrentSpriteIndex = 3;
					}
				}
			}
		}
	}

	void SpriteDemoManager::Render(const StepTimer& /* timer */)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		const uint32_t stride = VertexPositionTexture::VertexSize();
		const uint32_t offset = 0;
		const auto vertexBuffers = mVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.get(), nullptr, 0);

		winrt::impl::abi_t<ID3D11ShaderResourceView>* psShaderResources = nullptr;

		psShaderResources = mSpriteSheet.get();
		direct3DDeviceContext->PSSetShaderResources(0, 1, &psShaderResources);

		const auto vsConstantBuffers = mVSCBufferPerObject.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);

		const auto textureSamplers = mTextureSampler.get();
		direct3DDeviceContext->PSSetSamplers(0, 1, &textureSamplers);
		direct3DDeviceContext->OMSetBlendState(mAlphaBlending.get(), 0, 0xFFFFFFFF);

		direct3DDeviceContext->PSSetShaderResources(0, 1, &psShaderResources);

		DrawSprite(*mSprites[mCurrentSpriteIndex]);
	}

	void SpriteDemoManager::DrawSprite(Sprite& sprite)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();

		DirectX::XMMATRIX ProjectionMatrix = mCamera->ProjectionMatrix();
		DX::Transform2D Transform = sprite.Transform();

		if (mType == Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS)
		{
			auto instance = StateManager::GetInstance();
			auto state = instance->getState();
			auto activePlayer = instance->getActivePlayers();
			if (state == StateManager::GameState::MAIN_MENU && activePlayer == StateManager::ActivePlayers::PLAYER_ONE)
			{
				Transform = mSpriteInitialPositionsLookup.at(SpriteInitialPositions::MENU_PLAYER_ONE_BALLOON);
			}
			if (state == StateManager::GameState::MAIN_MENU && activePlayer == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
			{
				Transform = mSpriteInitialPositionsLookup.at(SpriteInitialPositions::MENU_PLAYER_TWO_BALLOON);
			}
		}

		if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE)
		{
			if (mPlayerMoveDirection == PlayerMoveDirection::RIGHT)
			{
				ProjectionMatrix = XMMatrixMultiply(ProjectionMatrix, XMMatrixScaling(-1, 1, 1));
				auto position = Transform.Position();
				Transform.SetPosition(-position.x - mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
			if (mPlayerMoveDirection == PlayerMoveDirection::LEFT)
			{
				auto position = Transform.Position();
				Transform.SetPosition(position.x + mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
		}
		if (mType == Sprite::SpriteTypeEnum::ENEMY_ONE)
		{
			if (mPlayerMoveDirection == PlayerMoveDirection::RIGHT)
			{
				ProjectionMatrix = XMMatrixMultiply(ProjectionMatrix, XMMatrixScaling(-1, 1, 1));
				auto position = Transform.Position();
				Transform.SetPosition(-position.x - mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
			if (mPlayerMoveDirection == PlayerMoveDirection::LEFT)
			{
				auto position = Transform.Position();
				Transform.SetPosition(position.x + mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
		}
		if (mType == Sprite::SpriteTypeEnum::ENEMY_TWO)
		{
			if (mPlayerMoveDirection == PlayerMoveDirection::RIGHT)
			{
				ProjectionMatrix = XMMatrixMultiply(ProjectionMatrix, XMMatrixScaling(-1, 1, 1));
				auto position = Transform.Position();
				Transform.SetPosition(-position.x - mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
			if (mPlayerMoveDirection == PlayerMoveDirection::LEFT)
			{
				auto position = Transform.Position();
				Transform.SetPosition(position.x + mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
		}
		if (mType == Sprite::SpriteTypeEnum::ENEMY_THREE)
		{
			if (mPlayerMoveDirection == PlayerMoveDirection::RIGHT)
			{
				ProjectionMatrix = XMMatrixMultiply(ProjectionMatrix, XMMatrixScaling(-1, 1, 1));
				auto position = Transform.Position();
				Transform.SetPosition(-position.x - mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
			if (mPlayerMoveDirection == PlayerMoveDirection::LEFT)
			{
				auto position = Transform.Position();
				Transform.SetPosition(position.x + mPlayerMovement.x, position.y + mPlayerMovement.y);
				Transform.SetRotation(0.0f);
				Transform.SetScale(SpriteScale);
			}
		}

		DirectX::XMMATRIX ViewProjectionMatrix = XMMatrixMultiply(mCamera->ViewMatrix(), ProjectionMatrix);
		const XMMATRIX wvp = XMMatrixTranspose(Transform.WorldMatrix() * ViewProjectionMatrix);
		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);

		XMMATRIX textureTransform = XMLoadFloat4x4(&sprite.TextureTransform());
		XMStoreFloat4x4(&mVSCBufferPerObjectData.TextureTransform, XMMatrixTranspose(textureTransform));

		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void SpriteDemoManager::InitializeVertices()
	{
		const VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)),
			VertexPositionTexture(XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)),
			VertexPositionTexture(XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)),
		};

		mVertexBuffer = nullptr;
		VertexPositionTexture::CreateVertexBuffer(not_null<ID3D11Device*>(mDeviceResources->GetD3DDevice()), vertices, not_null<ID3D11Buffer * *>(mVertexBuffer.put()));

		// Create and index buffer
		const uint16_t indices[] =
		{
			0, 1, 2,
			0, 2, 3
		};

		mIndexCount = narrow<uint32_t>(std::size(indices));

		D3D11_BUFFER_DESC indexBufferDesc{ 0 };
		indexBufferDesc.ByteWidth = sizeof(uint16_t) * mIndexCount;
		indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexSubResourceData{ 0 };
		indexSubResourceData.pSysMem = indices;
		mIndexBuffer = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.put()));
	}

	void SpriteDemoManager::InitializeSprites(Sprite::SpriteTypeEnum type, std::vector<std::shared_ptr<Sprite>>& sprites)
	{
		const XMFLOAT2 neighborOffset(2.0f, 2.0f);
		uint32_t spriteIndex = 0;
		for (uint32_t column = 0; column < mSpriteRowColumnLookupValuesByType.at(type).SpriteColumnCount; ++column)
		{
			for (uint32_t row = 0; row < mSpriteRowColumnLookupValuesByType.at(type).SpriteRowCount; ++row, ++spriteIndex)
			{
				// Set Position of the Sprite. We can default it to zero ( Currently Calculated based on row, column & neighborOffset )
				XMFLOAT2 position(mPosition.x, mPosition.y/*mPosition.x + column * neighborOffset.x * SpriteScale.x, mPosition.y + row * neighborOffset.y * SpriteScale.y*/);
				Transform2D transform(position, 0.0f, SpriteScale);
				auto sprite = make_shared<Sprite>(spriteIndex, transform, type);

				// Set Position of the Texture for specific Sprite. We use UV scaling to slap appropriate texture for sprite.
				XMFLOAT4X4 textureTransform;
				XMMATRIX textureTransformMatrix = XMMatrixScaling(UVScalingFactor.x, UVScalingFactor.y, 0) * XMMatrixTranslation(UVScalingFactor.x * sprite->SpriteIndex(), /*UVScalingFactor.y * sprite->SpriteIndex()*/ 0, 0.0f);
				XMStoreFloat4x4(&textureTransform, textureTransformMatrix);
				sprite->SetTextureTransform(textureTransform);

				// Finally we store sprite into our array
				sprites.push_back(move(sprite));
			}
		}
		(void)0;
	}

	void SpriteDemoManager::CalculateCurrentPosition()
	{
		switch (mType)
		{
		case Sprite::SpriteTypeEnum::PLAYER_ONE:
		case Sprite::SpriteTypeEnum::PLAYER_TWO:
		case Sprite::SpriteTypeEnum::ENEMY_ONE:
		case Sprite::SpriteTypeEnum::ENEMY_TWO:
		case Sprite::SpriteTypeEnum::ENEMY_THREE:
			PlayerPosX = PlayerPos.x + mPlayerMovement.x;
			PlayerPosY = PlayerPos.y + mPlayerMovement.y;
			break;
		default:
			break;
		}
	}
	void SpriteDemoManager::UpdateEnemyMovements()
	{
		UpdateEnemyXMovement();
		UpdateEnemyYMovement();
	}
	void SpriteDemoManager::UpdateEnemyXMovement()
	{
		if (EnemyMoveForceX < 1 && EnemyMoveForceX > -1 || EnemyMoveForceX > 15 || EnemyMoveForceX < -15)
		{
			Random r;
			switch (r.getRangedRandom(0, 5))
			{
			case 0:
				EnemyMoveForceX = -5;
				break;
			case 1:
				EnemyMoveForceX = 2;
				break;
			case 2:
				EnemyMoveForceX = -3;
				break;
			case 3:
				EnemyMoveForceX = 5;
				break;
			case 4:
				EnemyMoveForceX = -4;
				break;
			case 5:
				EnemyMoveForceX = 5;
				break;
			}
		}

		if (mPlayerState == PlayerState::FLYING)
		{
			// Simply flip Sprite based on current orientation
			if (mPlayerMoveDirection == PlayerMoveDirection::LEFT && EnemyMoveForceX > 0)
			{
				mPlayerMoveDirection = PlayerMoveDirection::RIGHT;
			}
			if (mPlayerMoveDirection == PlayerMoveDirection::RIGHT && EnemyMoveForceX < 0)
			{
				mPlayerMoveDirection = PlayerMoveDirection::LEFT;
			}
		}
		if (mPlayerState != PlayerState::DEAD)
		{
			mPlayerMovement.x += (float)EnemyMoveForceX/100;
			EnemyMoveForceX -= 0.05;

			// If an Enemy goes out of screen, make him appear entering from other side.
			if (PlayerPosX > 45)
			{
				PlayerPosX = -44;
				mPlayerMovement.x = PlayerPosX - mPosition.x;
			}
			if (PlayerPosX < -45)
			{
				PlayerPosX = 44;
				mPlayerMovement.x = PlayerPosX - mPosition.x;
			}
		}
	}

	void SpriteDemoManager::UpdateEnemyYMovement()
	{
		if ((EnemyMoveForceY < 1 && EnemyMoveForceY > -1) || EnemyMoveForceY > 5 || EnemyMoveForceY < -5)
		{
			Random r;
			switch (r.getRangedRandom(0, 5))
			{
			case 0:
				EnemyMoveForceY = 1;
				break;
			case 1:
				EnemyMoveForceY = -2;
				break;
			case 2:
				EnemyMoveForceY = 4;
				break;
			case 3:
				EnemyMoveForceY = -5;
				break;
			case 4:
				EnemyMoveForceY = 5;
				break;
			case 5:
				EnemyMoveForceY = -2;
				break;
			}
		}

		if (mPlayerState != PlayerState::DEAD)
		{
			mPlayerMovement.y += (float)EnemyMoveForceY / 100;
			EnemyMoveForceY -= 0.05;

			// Don't let enemies go inside water unless they are dead.
			if (PlayerPosY < -37)
			{
				PlayerPosY = -36;
				mPlayerMovement.y += 1;
				EnemyMoveForceY = 80;
			}

			// Also make sure that enemies don't go out of top-bound.
			// make top bound for enemies smaller than players so player can have advantage going over top.
			if (PlayerPosY > 40)
			{
				PlayerPosY = 39;
				mPlayerMovement.y -= 1;
				EnemyMoveForceY = -5;
			}
		}
	}
}