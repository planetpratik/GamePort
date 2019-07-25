#include "imgui_impl_uwp.h"

static IUnknown* g_Window = nullptr;
static ImVec2 g_DisplaySize;

IMGUI_IMPL_API bool ImGui_ImplUwp_Init(IUnknown* window, int displayWidth, int displayHeight)
{
	// Setup back-end capabilities flags
	g_Window = window;
	ImGuiIO& io = ImGui::GetIO();
	//io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	//io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.BackendPlatformName = "imgui_impl_uwp";
	io.ImeWindowHandle = window;
	
	g_DisplaySize.x = static_cast<float>(displayWidth);
	g_DisplaySize.y = static_cast<float>(displayHeight);

	return true;
}

IMGUI_IMPL_API void ImGui_ImplUwp_Shutdown()
{
	g_Window = nullptr;
}

IMGUI_IMPL_API void ImGui_ImplUwp_NewFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");
	io.DisplaySize = g_DisplaySize;
}
