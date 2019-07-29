#include "pch.h"
#include "SpriteDemoManager.h"
#include "StepTimer.h"
#include "VertexDeclarations.h"
#include "Utility.h"
#include "Camera.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	const std::unordered_map<Sprite::SpriteTypeEnum, SpriteDemoManager::RowColumnLookupInfo> SpriteDemoManager::mSpriteRowColumnLookupValuesByType =
	{
		{ Sprite::SpriteTypeEnum::MAIN_MENU_SCREEN, {1u, 1u, {0.0, 0.0}, {50, 50}, {1.0f, 1.0f}, L"Content\\Textures\\StartScreen.png"}},
		{ Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS, {1u, 4u, {-20.0f, -10.0f}, {2, 4}, {1.0f/4, 1.0f}, L"Content\\Textures\\StartScreenBalloons.png"}},
		{ Sprite::SpriteTypeEnum::LEVEL_SCREEN, {1u, 1u, {0.0, 0.0}, {50, 50}, {1.0f, 1.0f}, L"Content\\Textures\\Level.png"}},
		{ Sprite::SpriteTypeEnum::PLAYER_ONE, {7u, 10u, {-40.0, -35.0}, {2, 3.2}, {1.0f/9, 1.0f}, L"Content\\Textures\\Player_One.png"}},
	};

	const std::unordered_map<SpriteDemoManager::SpriteInitialPositions, DirectX::XMFLOAT2> SpriteDemoManager::mSpriteInitialPositionsLookup =
	{
		{ SpriteDemoManager::SpriteInitialPositions::MENU_PLAYER_ONE_BALLOON, DirectX::XMFLOAT2(-20.0f, -10.0f)},
		{ SpriteDemoManager::SpriteInitialPositions::MENU_PLAYER_TWO_BALLOON, DirectX::XMFLOAT2(0.0f, 0.0f)},
		{ SpriteDemoManager::SpriteInitialPositions::PLAYER_ONE, DirectX::XMFLOAT2(-40.0f, -35.0f)},
	};

	SpriteDemoManager::SpriteDemoManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera, Sprite::SpriteTypeEnum type) :
		DrawableGameComponent(deviceResources, camera), mType(type), mCurrentSpriteIndex(0), mSpriteInitialPositions(SpriteInitialPositions::UNDEFINED)
	{
		SpriteScale = mSpriteRowColumnLookupValuesByType.at(type).SpriteScale;
		UVScalingFactor = mSpriteRowColumnLookupValuesByType.at(type).UVScalingFactor;
		mPosition = mSpriteRowColumnLookupValuesByType.at(type).Position;
		SpriteSheetName = mSpriteRowColumnLookupValuesByType.at(type).SpriteSheetName;
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

	void SpriteDemoManager::UpdateData(const StepTimer& timer, StateManager::ActivePlayers activePlayers)
	{
		if (timer.GetTotalSeconds() > mLastDataUpdateTime + DataUpdateDelay)
		{
			mLastDataUpdateTime = timer.GetTotalSeconds();
			auto instance = StateManager::GetInstance();
			auto state = instance->getState();
			//auto activePlayer = instance->getActivePlayers();
			// If currently in the Main Menu
			if (state == StateManager::GameState::MAIN_MENU)
			{
				if (mType == Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS && activePlayers == StateManager::ActivePlayers::PLAYER_ONE)
				{
					// set transform location for current sprite 
					mSprites[mCurrentSpriteIndex]->SetTransform(mSpriteInitialPositionsLookup.at(SpriteInitialPositions::MENU_PLAYER_ONE_BALLOON));
				}
				if (mType == Sprite::SpriteTypeEnum::MAIN_MENU_BALLOONS && activePlayers == StateManager::ActivePlayers::PLAYER_ONE_AND_TWO)
				{
					// set transform location for current sprite 
					mSprites[mCurrentSpriteIndex]->SetTransform(mSpriteInitialPositionsLookup.at(SpriteInitialPositions::MENU_PLAYER_TWO_BALLOON));
				}
			}

			// If Player(s) is/are in the game.
			if (state == StateManager::GameState::GAME_STARTED)
			{
				
			}

		}

	}

	void SpriteDemoManager::Update(const StepTimer& timer)
	{
		if (timer.GetTotalSeconds() > mLastAnimationUpdateTime + AnimationUpdateDelay)
		{
			mLastAnimationUpdateTime = timer.GetTotalSeconds();
			auto instance = StateManager::GetInstance();
			auto state = instance->getState();
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
			if (state == StateManager::GameState::GAME_STARTED)
			{
			}
		}
		

		//if (timer.GetTotalSeconds() > mLastMoodUpdateTime + MoodUpdateDelay)
		//{
		//	mLastMoodUpdateTime = timer.GetTotalSeconds();

		//	uint32_t spritesToChange = mSpriteCountDistribution(mRandomGenerator);
		//	for (uint32_t i = 0; i < spritesToChange; ++i)
		//	{
		//		uint32_t spriteIndex = mSpriteCountDistribution(mRandomGenerator);
		//		spriteIndex;
		//		//auto sprite = mSprites[spriteIndex];
		//		//ChangeMood(*sprite);
		//	}
		//}
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

		/*for (const auto& sprite : mSprites)
		{
			DrawSprite(*sprite);
		}*/
	}

	void SpriteDemoManager::DrawSprite(MoodySprite& sprite)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		
		const XMMATRIX wvp = XMMatrixTranspose(sprite.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);
		XMMATRIX textureTransform = XMLoadFloat4x4(&sprite.TextureTransform());
		XMStoreFloat4x4(&mVSCBufferPerObjectData.TextureTransform, XMMatrixTranspose(textureTransform));		 
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void SpriteDemoManager::DrawSprite(int/*winrt::com_ptr<ID3D11Texture2D>*/ /*backgroundImageSprite*/)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		XMFLOAT2 position(mPosition.x, mPosition.y);
		Transform2D transform(position, 0.0f, BackgroundImageScale);
		const XMMATRIX wvp = XMMatrixTranspose(transform.WorldMatrix() * mCamera->ViewProjectionMatrix());
		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);
		XMMATRIX textureTransform = XMLoadFloat4x4(&DX::MatrixHelper::Identity);
		XMStoreFloat4x4(&mVSCBufferPerObjectData.TextureTransform, XMMatrixTranspose(textureTransform));
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}

	void SpriteDemoManager::DrawSprite(Sprite& sprite)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();

		DirectX::XMMATRIX ProjectionMatrix = mCamera->ProjectionMatrix();
		if (mType == Sprite::SpriteTypeEnum::PLAYER_ONE)
		{
			//ProjectionMatrix = XMMatrixMultiply(ProjectionMatrix,XMMatrixScaling(1, -1, 1));
		}
		DirectX::XMMATRIX ViewProjectionMatrix = XMMatrixMultiply(mCamera->ViewMatrix(), ProjectionMatrix);

		const XMMATRIX wvp = XMMatrixTranspose(sprite.Transform().WorldMatrix() * ViewProjectionMatrix/*mCamera->ViewProjectionMatrix()*/);
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
		VertexPositionTexture::CreateVertexBuffer(not_null<ID3D11Device*>(mDeviceResources->GetD3DDevice()), vertices, not_null<ID3D11Buffer**>(mVertexBuffer.put()));

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
				XMFLOAT2 position(mPosition.x,mPosition.y/*mPosition.x + column * neighborOffset.x * SpriteScale.x, mPosition.y + row * neighborOffset.y * SpriteScale.y*/);
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

	void SpriteDemoManager::ChangeMood(MoodySprite& sprite)
	{
		MoodySprite::Moods mood = GetRandomMood();

		XMFLOAT4X4 textureTransform;
		XMMATRIX textureTransformMatrix = XMMatrixScaling(UVScalingFactor.x, UVScalingFactor.y, 0) * XMMatrixTranslation(UVScalingFactor.x * sprite.SpriteIndex(), UVScalingFactor.y * static_cast<uint32_t>(mood), 0.0f);
		XMStoreFloat4x4(&textureTransform, textureTransformMatrix);
		sprite.SetTextureTransform(textureTransform);
	}

	MoodySprite::Moods SpriteDemoManager::GetRandomMood()
	{
		return static_cast<MoodySprite::Moods>(mMoodDistribution(mRandomGenerator));
	}
}