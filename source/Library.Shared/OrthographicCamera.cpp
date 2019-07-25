#include "pch.h"
#include "OrthographicCamera.h"

using namespace std;
using namespace DirectX;

namespace DX
{
	OrthographicCamera::OrthographicCamera(const shared_ptr<DX::DeviceResources>& deviceResourcese, float viewWidth, float viewHeight, float nearPlaneDistance, float farPlaneDistance) :
		Camera(deviceResourcese, nearPlaneDistance, farPlaneDistance),
		mViewWidth(viewWidth), mViewHeight(viewHeight)
    {
    }
    
    void OrthographicCamera::UpdateProjectionMatrix()
    {
		if (mProjectionMatrixDataDirty)
		{
			XMMATRIX projectionMatrix = XMMatrixOrthographicRH(mViewWidth, mViewHeight, mNearPlaneDistance, mFarPlaneDistance);
			XMStoreFloat4x4(&mProjectionMatrix, projectionMatrix);
		}
    }
}
