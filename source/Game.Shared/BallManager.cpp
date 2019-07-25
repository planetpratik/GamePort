#include "pch.h"
#include "BallManager.h"
#include "Ball.h"
#include "VectorHelper.h"
#include "VertexDeclarations.h"
#include "Camera.h"
#include "Utility.h"
#include "ColorHelper.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	BallManager::BallManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(deviceResources, camera)
	{
	}

	std::shared_ptr<Field> BallManager::ActiveField() const
	{
		return mActiveField;
	}

	void BallManager::SetActiveField(const shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void BallManager::CreateDeviceDependentResources()
	{
		auto compiledVertexShader = Utility::LoadBinaryFile(L"Content\\Shaders\\ShapeRendererVS.cso");
		mVertexShader = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateVertexShader(compiledVertexShader.data(), compiledVertexShader.size(), nullptr, mVertexShader.put()), "ID3D11Device::CreatedVertexShader() failed.");
		mInputLayout = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateInputLayout(VertexPosition::InputElements.data(), narrow_cast<uint32_t>(VertexPosition::InputElements.size()), compiledVertexShader.data(), compiledVertexShader.size(), mInputLayout.put()), "ID3D11Device::CreateInputLayout() failed.");

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");

		auto compiledPixelShader = Utility::LoadBinaryFile(L"Content\\Shaders\\ShapeRendererPS.cso");
		mPixelShader = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreatePixelShader(compiledPixelShader.data(), compiledPixelShader.size(), nullptr, mPixelShader.put()), "ID3D11Device::CreatedPixelShader() failed.");
		constantBufferDesc.ByteWidth = sizeof(XMFLOAT4);
		mPSCBufferPerObject = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerObject.put()), "ID3D11Device::CreateBuffer() failed.");

		InitializeLineVertices();
		InitializeTriangleVertices();
		InitializeBalls();
	}

	void BallManager::ReleaseDeviceDependentResources()
	{
		mVertexShader = nullptr;
		mPixelShader = nullptr;
		mInputLayout = nullptr;
		mLineVertexBuffer = nullptr;
		mTriangleVertexBuffer = nullptr;
		mVSCBufferPerObject = nullptr;
		mPSCBufferPerObject = nullptr;
	}

	void BallManager::Update(const StepTimer& timer)
	{
		for (const auto& ball : mBalls)
		{
			ball->Update(timer);
		}
	}

	void BallManager::Render(const StepTimer& /*timer*/)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		direct3DDeviceContext->VSSetShader(mVertexShader.get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.get(), nullptr, 0);

		const auto vsConstantBuffers = mVSCBufferPerObject.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);

		const auto psConstantBuffers = mPSCBufferPerObject.get();
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, &psConstantBuffers);

		for (const auto& ball : mBalls)
		{
			if (ball->IsSolid())
			{
				DrawSolidBall(*ball);
			}
			else
			{
				DrawBall(*ball);
			}
		}
	}

	void BallManager::DrawBall(const Ball& ball)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

		const uint32_t stride = VertexPosition::VertexSize();
		const uint32_t offset = 0;
		const auto vertexBuffers = mLineVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);

		const XMMATRIX wvp = XMMatrixTranspose(XMMatrixScaling(ball.Radius(), ball.Radius(), ball.Radius()) * ball.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.get(), 0, nullptr, wvp.r, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.get(), 0, nullptr, &ball.Color(), 0, 0);

		direct3DDeviceContext->Draw(LineCircleVertexCount, 0);
	}

	void BallManager::DrawSolidBall(const Ball & ball)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		const uint32_t stride = VertexPosition::VertexSize();
		const uint32_t offset = 0;
		const auto vertexBuffers = mTriangleVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);

		const XMMATRIX wvp = XMMatrixTranspose(XMMatrixScaling(ball.Radius(), ball.Radius(), ball.Radius()) * ball.Transform().WorldMatrix() * mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.get(), 0, nullptr, wvp.r, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.get(), 0, nullptr, &ball.Color(), 0, 0);

		direct3DDeviceContext->Draw(SolidCircleVertexCount, 0);
	}

	void BallManager::InitializeLineVertices()
	{
		const float increment = XM_2PI / CircleResolution;

		VertexPosition vertices[LineCircleVertexCount];

		for (int i = 0; i < CircleResolution; i++)
		{
			VertexPosition& vertex = vertices[i];

			vertex.Position.x = cosf(i * increment);
			vertex.Position.y = sinf(i * increment);
			vertex.Position.z = 0.0f;
			vertex.Position.w = 1.0f;
		}

		// Closing line to complete the circle
		vertices[CircleResolution] = VertexPosition(vertices[0]);

		// Axis line for visualizing rotation
		vertices[CircleResolution + 1] = VertexPosition(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		
		mLineVertexBuffer = nullptr;
		VertexPosition::CreateVertexBuffer(not_null<ID3D11Device*>(mDeviceResources->GetD3DDevice()), vertices, not_null<ID3D11Buffer**>(mLineVertexBuffer.put()));
	}

	void BallManager::InitializeTriangleVertices()
	{
		const float increment = XM_2PI / CircleResolution;
		const XMFLOAT4 center(0.0f, 0.0f, 0.0f, 1.0f);

		vector<VertexPosition> vertices;
		vertices.reserve(SolidCircleVertexCount);
		for (int i = 0; i <= CircleResolution; i++)
		{
			VertexPosition vertex;
			vertex.Position.x = cosf(i * increment);
			vertex.Position.y = sinf(i * increment);
			vertex.Position.z = 0.0f;
			vertex.Position.w = 1.0f;

			vertices.push_back(vertex);
			vertices.push_back(center);
		}

		assert(vertices.size() == SolidCircleVertexCount);
		
		mTriangleVertexBuffer = nullptr;
		VertexPosition::CreateVertexBuffer(not_null<ID3D11Device*>(mDeviceResources->GetD3DDevice()), vertices, not_null<ID3D11Buffer**>(mTriangleVertexBuffer.put()));
	}

	void BallManager::InitializeBalls()
	{
		random_device device;
		default_random_engine generator(device());

		const float minVelocity = -30.0f;
		const float maxVelocity = 30.0f;
		uniform_real_distribution<float> velocityDistribution(minVelocity, maxVelocity);
		uniform_int_distribution<uint32_t> isSolidDistribution(0, 1);

		uniform_real_distribution<float> rotationDistribution(0, XM_2PI);

		const float minRadius = 0.1f;
		const float maxRadius = 5.0f;
		uniform_real_distribution<float> radiusDistribution(minRadius, maxRadius);

		const uint32_t ballCount = 30;
		for (uint32_t i = 0; i < ballCount; ++i)
		{
			const float rotation = rotationDistribution(generator);
			const float radius = radiusDistribution(generator);
			const XMFLOAT4 color = ColorHelper::RandomColor();
			const XMFLOAT2 velocity(velocityDistribution(generator), velocityDistribution(generator));
			const bool isSolid = isSolidDistribution(generator) < 1;
			mBalls.emplace_back(make_shared<Ball>(*this, Transform2D(Vector2Helper::Zero, rotation), radius, color, velocity, isSolid));
		}
	}
}
