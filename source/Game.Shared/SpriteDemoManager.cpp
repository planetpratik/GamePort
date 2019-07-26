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
	SpriteDemoManager::SpriteDemoManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera, uint32_t spriteRowCount, uint32_t spriteColumCount) :
		DrawableGameComponent(deviceResources, camera),
		mSpriteRowCount(spriteRowCount), mSpriteColumnCount(spriteColumCount),
		mRandomGenerator(mRandomDevice()),
		mSpriteDistribution(0, SpriteCount - 1),
		mMoodDistribution(static_cast<uint32_t>(MoodySprite::Moods::Neutral), static_cast<uint32_t>(MoodySprite::Moods::Angry))
	{
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
		
		/*mSpriteSheet = nullptr;
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mDeviceResources->GetD3DDevice(), L"Content\\Textures\\snoods_default.png", nullptr, mSpriteSheet.put()));
		InitializeVertices();
		InitializeSprites();
		mSpriteCountDistribution = uniform_int_distribution<uint32_t>(0U, static_cast<uint32_t>(mSprites.size()) - 1);*/

		mSpriteSheetMainMenu = nullptr;
		ThrowIfFailed(DirectX::CreateWICTextureFromFile(mDeviceResources->GetD3DDevice(), L"Content\\Textures\\StartScreen.png", nullptr, mSpriteSheetMainMenu.put()));
		InitializeVertices();
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
	}

	void SpriteDemoManager::Update(const StepTimer& /*timer*/)
	{		
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

		if (!mIsGameStarted)
		{
			psShaderResources = mSpriteSheetMainMenu.get();
		}
		else
		{
			// Use different Sprite Sheets When game starts.
		}
		//const auto psShaderResources = mSpriteSheet.get();
		direct3DDeviceContext->PSSetShaderResources(0, 1, &psShaderResources);

		const auto vsConstantBuffers = mVSCBufferPerObject.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);
		
		const auto textureSamplers = mTextureSampler.get();
		direct3DDeviceContext->PSSetSamplers(0, 1, &textureSamplers);
		direct3DDeviceContext->OMSetBlendState(mAlphaBlending.get(), 0, 0xFFFFFFFF);

		DrawSprite(MAIN_MENU_BACKGROUND_IMAGE_INDEX);

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

	void SpriteDemoManager::InitializeSprites()
	{	
		const XMFLOAT2 neighborOffset(2.0f, 2.0f);
		for (uint32_t column = 0; column < mSpriteColumnCount; ++column)		
		{
			for (uint32_t row = 0; row < mSpriteRowCount; ++row)
			{
				XMFLOAT2 position(mPosition.x + column * neighborOffset.x * SpriteScale.x, mPosition.y + row * neighborOffset.y * SpriteScale.y);
				Transform2D transform(position, 0.0f, SpriteScale);								
				uint32_t spriteIndex = mSpriteDistribution(mRandomGenerator);
				auto sprite = make_shared<MoodySprite>(spriteIndex, transform);
				ChangeMood(*sprite);
				mSprites.push_back(move(sprite));
			}
		}
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