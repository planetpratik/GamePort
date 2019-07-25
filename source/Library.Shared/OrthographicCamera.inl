namespace DX
{
	inline float OrthographicCamera::ViewWidth() const
	{
		return mViewWidth;
	}

	inline void OrthographicCamera::SetViewWidth(float viewWidth)
	{
		if (viewWidth > 0.0f)
		{
			mViewWidth = viewWidth;
			mProjectionMatrixDataDirty = true;
		}
	}

	inline float OrthographicCamera::ViewHeight() const
	{
		return mViewHeight;
	}

	inline void OrthographicCamera::SetViewHeight(float viewHeight)
	{
		if (viewHeight > 0.0f)
		{
			mViewHeight = viewHeight;
			mProjectionMatrixDataDirty = true;
		}
	}
}