#include "pch.h"
#include "Camera.h"
#include "VectorHelper.h"

using namespace std;
using namespace DirectX;

namespace DX
{
	Camera::Camera(const shared_ptr<DX::DeviceResources>& deviceResources, float nearPlaneDistance, float farPlaneDistance) :
		GameComponent(deviceResources),
		mNearPlaneDistance(nearPlaneDistance), mFarPlaneDistance(farPlaneDistance)
	{		
		Reset();
	}	

	XMMATRIX Camera::ViewProjectionMatrix() const
	{
		XMMATRIX viewMatrix = XMLoadFloat4x4(&mViewMatrix);
		XMMATRIX projectionMatrix = XMLoadFloat4x4(&mProjectionMatrix);

		return XMMatrixMultiply(viewMatrix, projectionMatrix);
	}

	void Camera::SetPosition(float x, float y, float z)
	{
		XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
		SetPosition(position);
	}

	void Camera::SetPosition(FXMVECTOR position)
	{
		XMStoreFloat3(&mPosition, position);
		mViewMatrixDataDirty = true;
	}

	void Camera::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;
		mViewMatrixDataDirty = true;
	}

	void Camera::CreateDeviceDependentResources()
	{
		UpdateProjectionMatrix();		
	}

	void Camera::Reset()
	{
		mPosition = Vector3Helper::Zero;
		mDirection = Vector3Helper::Forward;
		mUp = Vector3Helper::Up;
		mRight = Vector3Helper::Right;
		mViewMatrixDataDirty = true;

		UpdateViewMatrix();
	}

	void Camera::Update(const StepTimer& /* timer */)
	{
		if (mViewMatrixDataDirty)
		{
			UpdateViewMatrix();
		}
	}

	void Camera::UpdateViewMatrix()
	{
		XMVECTOR eyePosition = XMLoadFloat3(&mPosition);
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR upDirection = XMLoadFloat3(&mUp);

		XMMATRIX viewMatrix = XMMatrixLookToRH(eyePosition, direction, upDirection);
		XMStoreFloat4x4(&mViewMatrix, viewMatrix);

		mViewMatrixDataDirty = false;
	}

	void Camera::ApplyRotation(CXMMATRIX transform)
	{
		XMVECTOR direction = XMLoadFloat3(&mDirection);
		XMVECTOR up = XMLoadFloat3(&mUp);

		direction = XMVector3TransformNormal(direction, transform);
		direction = XMVector3Normalize(direction);

		up = XMVector3TransformNormal(up, transform);
		up = XMVector3Normalize(up);

		XMVECTOR right = XMVector3Cross(direction, up);
		up = XMVector3Cross(right, direction);

		XMStoreFloat3(&mDirection, direction);
		XMStoreFloat3(&mUp, up);
		XMStoreFloat3(&mRight, right);

		mViewMatrixDataDirty = true;
	}

	void Camera::ApplyRotation(const XMFLOAT4X4& transform)
	{
		XMMATRIX transformMatrix = XMLoadFloat4x4(&transform);
		ApplyRotation(transformMatrix);
	}
}
