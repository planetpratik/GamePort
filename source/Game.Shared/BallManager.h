#pragma once

#include "DrawableGameComponent.h"
#include <DirectXMath.h>
#include <vector>
#include <winrt/base.h>

namespace DirectXGame
{
	class Ball;
	class Field;

	class BallManager final : public DX::DrawableGameComponent
	{
	public:
		BallManager(const std::shared_ptr<DX::DeviceResources>& deviceResources, const std::shared_ptr<DX::Camera>& camera);

		std::shared_ptr<Field> ActiveField() const;
		void SetActiveField(const std::shared_ptr<Field>& field);

		virtual void CreateDeviceDependentResources() override;
		virtual void ReleaseDeviceDependentResources() override;
		virtual void Update(const DX::StepTimer& timer) override;
		virtual void Render(const DX::StepTimer& timer) override;

	private:
		void InitializeLineVertices();
		void InitializeTriangleVertices();
		void InitializeBalls();
		void DrawBall(const Ball& ball);
		void DrawSolidBall(const Ball& ball);

		inline static const std::uint32_t CircleResolution{ 32 };
		inline static const std::uint32_t LineCircleVertexCount{ BallManager::CircleResolution + 2 };
		inline static const std::uint32_t SolidCircleVertexCount{ (BallManager::CircleResolution + 1) * 2 };

		winrt::com_ptr<ID3D11VertexShader> mVertexShader;
		winrt::com_ptr<ID3D11PixelShader> mPixelShader;
		winrt::com_ptr<ID3D11InputLayout> mInputLayout;
		winrt::com_ptr<ID3D11Buffer> mLineVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mTriangleVertexBuffer;
		winrt::com_ptr<ID3D11Buffer> mVSCBufferPerObject;
		winrt::com_ptr<ID3D11Buffer> mPSCBufferPerObject;
		std::vector<std::shared_ptr<Ball>> mBalls;
		std::shared_ptr<Field> mActiveField;
	};
}

