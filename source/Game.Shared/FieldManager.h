#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>
#include <wrl/client.h>

namespace DirectXGame
{
	class Field;

	class FieldManager final : public DX::DrawableGameComponent
	{
	public:
		FieldManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);
				
		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Render(const DX::StepTimer& timer) override;

	private:
		void DrawField(const Field& field);

		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mIndexBuffer;
		winrt::com_ptr<ID3D11Buffer> mVSCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPSCBufferPerObject;
		std::uint32_t mIndexCount = 0;
		std::shared_ptr<Field> mActiveField;
	};
}