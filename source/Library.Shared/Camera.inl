namespace DX
{
	inline const DirectX::XMFLOAT3& Camera::Position() const
	{
		return mPosition;
	}

	inline const DirectX::XMFLOAT3& Camera::Direction() const
	{
		return mDirection;
	}

	inline const DirectX::XMFLOAT3& Camera::Up() const
	{
		return mUp;
	}

	inline const DirectX::XMFLOAT3& Camera::Right() const
	{
		return mRight;
	}

	inline DirectX::XMVECTOR Camera::PositionVector() const
	{
		return DirectX::XMLoadFloat3(&mPosition);
	}

	inline DirectX::XMVECTOR Camera::DirectionVector() const
	{
		return DirectX::XMLoadFloat3(&mDirection);
	}

	inline DirectX::XMVECTOR Camera::UpVector() const
	{
		return DirectX::XMLoadFloat3(&mUp);
	}

	inline DirectX::XMVECTOR Camera::RightVector() const
	{
		return DirectX::XMLoadFloat3(&mRight);
	}

	inline float Camera::NearPlaneDistance() const
	{
		return mNearPlaneDistance;
	}

	inline void Camera::SetNearPlaneDistance(float distance)
	{
		mNearPlaneDistance = distance;
		mProjectionMatrixDataDirty = true;
	}

	inline float Camera::FarPlaneDistance() const
	{
		return mFarPlaneDistance;
	}

	inline void Camera::SetFarPlaneDistance(float distance)
	{
		mFarPlaneDistance = distance;
		mProjectionMatrixDataDirty = true;
	}

	inline DirectX::XMMATRIX Camera::ViewMatrix() const
	{
		return DirectX::XMLoadFloat4x4(&mViewMatrix);
	}

	inline DirectX::XMMATRIX Camera::ProjectionMatrix() const
	{
		return DirectX::XMLoadFloat4x4(&mProjectionMatrix);
	}
}