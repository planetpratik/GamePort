#include "pch.h"
#include "ImGuiComponent.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_uwp.h"
#include <map>

using namespace std;
using namespace DX;

namespace DX
{
	ImGuiComponent::ImGuiComponent(const std::shared_ptr<DX::DeviceResources>& deviceResources, Styles style, bool useCustomDraw) :
		DrawableGameComponent(deviceResources), mStyle(style), mUseCustomDraw(useCustomDraw)
	{
	}

	ImGuiComponent::Styles ImGuiComponent::Style() const
	{
		return mStyle;
	}

	void ImGuiComponent::SetStyle(Styles style)
	{
		static const map<Styles, function<void(ImGuiStyle*)>> styleMap
		{
			{ Styles::Classic, ImGui::StyleColorsClassic },
			{ Styles::Light, ImGui::StyleColorsLight },
			{ Styles::Dark, ImGui::StyleColorsDark }
		};

		mStyle = style;
		styleMap.at(mStyle)(nullptr);
	}

	void ImGuiComponent::CreateDeviceDependentResources()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		auto displaySize = mDeviceResources->GetOutputSize();
		ImGui_ImplUwp_Init(mDeviceResources->GetWindow(), displaySize.right - displaySize.left, displaySize.bottom - displaySize.top);
		ImGui_ImplDX11_Init(mDeviceResources->GetD3DDevice(),mDeviceResources->GetD3DDeviceContext());
		SetStyle(mStyle);
	}

	void ImGuiComponent::ReleaseDeviceDependentResources()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplUwp_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiComponent::Render(const StepTimer&)
	{
		assert(ImGui::GetCurrentContext() != NULL);

		if (mUseCustomDraw == false)
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplUwp_NewFrame();
			ImGui::NewFrame();

			for (auto& block : mRenderBlocks)
			{
				(*block)();
			}

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}

	bool ImGuiComponent::UseCustomDraw() const
	{
		return mUseCustomDraw;
	}

	void ImGuiComponent::SetUseCustomDraw(bool useCustomDraw)
	{
		mUseCustomDraw = useCustomDraw;
	}

	void ImGuiComponent::CustomDraw()
	{
		assert(mUseCustomDraw);
		ImGui::Render();
	}

	const std::vector<std::shared_ptr<ImGuiComponent::RenderBlock>>& ImGuiComponent::RenderBlocks() const
	{
		return mRenderBlocks;
	}

	void ImGuiComponent::AddRenderBlock(std::shared_ptr<RenderBlock> block)
	{
		mRenderBlocks.push_back(block);
	}

	void ImGuiComponent::RemoveRenderBlock(std::shared_ptr<RenderBlock> block)
	{
		mRenderBlocks.erase(find(mRenderBlocks.cbegin(), mRenderBlocks.cend(), block));
	}
}