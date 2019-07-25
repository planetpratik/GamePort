#include "pch.h"
#include "FieldManager.h"
#include "Field.h"
#include "VectorHelper.h"
#include "VertexDeclarations.h"
#include "Camera.h"
#include "Utility.h"

using namespace std;
using namespace gsl;
using namespace DirectX;
using namespace DX;

namespace DirectXGame
{
	FieldManager::FieldManager(const shared_ptr<DX::DeviceResources>& deviceResources, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(deviceResources, camera)
	{
	}

	shared_ptr<Field> FieldManager::ActiveField() const
	{
		return mActiveField;
	}

	void FieldManager::SetActiveField(const std::shared_ptr<Field>& field)
	{
		mActiveField = field;
	}

	void FieldManager::CreateDeviceDependentResources()
	{
		// Create a field
		const XMFLOAT2 position = Vector2Helper::Zero;
		const XMFLOAT2 size(95, 80);
		const XMFLOAT4 color(Colors::AntiqueWhite.f);
		mActiveField = make_shared<Field>(position, size, color);		

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

		// Create a vertex buffer for rendering a box
		const uint32_t boxVertexCount = 4;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosition) * boxVertexCount, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		mVertexBuffer = nullptr;

		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, nullptr, mVertexBuffer.put()));

		// Create an index buffer for the box (line strip)
		const uint16_t indices[] =
		{
			0, 1, 2, 3, 0
		};

		mIndexCount = narrow<uint32_t>(std::size(indices));

		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(uint16_t) * mIndexCount, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE);
		D3D11_SUBRESOURCE_DATA indexSubResourceData = { 0 };
		indexSubResourceData.pSysMem = indices;
		mIndexBuffer = nullptr;
		ThrowIfFailed(mDeviceResources->GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, mIndexBuffer.put()));
	}

	void FieldManager::ReleaseDeviceDependentResources()
	{
		mVertexShader = nullptr;
		mPixelShader = nullptr;
		mInputLayout = nullptr;
		mVertexBuffer = nullptr;
		mVertexBuffer = nullptr;
		mVSCBufferPerObject = nullptr;
		mPSCBufferPerObject = nullptr;
	}

	void FieldManager::Render(const StepTimer &)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.get());

		const uint32_t stride = VertexPosition::VertexSize();
		const uint32_t offset = 0;
		const auto vertexBuffers = mVertexBuffer.get();
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffers, &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.get(), nullptr, 0);

		const XMMATRIX wvp = XMMatrixTranspose(mCamera->ViewProjectionMatrix());
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.get(), 0, nullptr, reinterpret_cast<const float*>(wvp.r), 0, 0);
		const auto vsConstantBuffers = mVSCBufferPerObject.get();
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &vsConstantBuffers);

		const auto psConstantBuffers = mPSCBufferPerObject.get();
		direct3DDeviceContext->PSSetConstantBuffers(0, 1, &psConstantBuffers);

		DrawField(*mActiveField);
	}

	void FieldManager::DrawField(const Field& field)
	{	
		const XMFLOAT2& position = field.Position();
		const XMFLOAT2& size = field.Size();
		const XMFLOAT2 halfSize(size.x / 2.0f, size.y / 2.0f);

		const VertexPosition vertices[] =
		{
			// Upper-Left
			VertexPosition(XMFLOAT4(position.x - halfSize.x, position.y + halfSize.y, 0.0f, 1.0f)),

			// Upper-Right
			VertexPosition(XMFLOAT4(position.x + halfSize.x, position.y + halfSize.y, 0.0f, 1.0f)),

			// Lower-Right
			VertexPosition(XMFLOAT4(position.x + halfSize.x, position.y - halfSize.y, 0.0f, 1.0f)),

			// Lower-Left
			VertexPosition(XMFLOAT4(position.x - halfSize.x, position.y - halfSize.y, 0.0f, 1.0f)),
		};

		ID3D11DeviceContext* direct3DDeviceContext = mDeviceResources->GetD3DDeviceContext();
		const uint32_t vertexCount = narrow_cast<uint32_t>(std::size(vertices));
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		ThrowIfFailed(direct3DDeviceContext->Map(mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource));
		memcpy(mappedSubResource.pData, vertices, sizeof(VertexPosition) * vertexCount);
		direct3DDeviceContext->Unmap(mVertexBuffer.get(), 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.get(), 0, nullptr, &field.Color(), 0, 0);
		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
	}
}