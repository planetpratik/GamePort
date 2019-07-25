#pragma once

#include "imgui/imgui.h"
#include <wrl/client.h>

IMGUI_IMPL_API bool     ImGui_ImplUwp_Init(IUnknown* window, int displayWidth, int displayHeight);
IMGUI_IMPL_API void     ImGui_ImplUwp_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplUwp_NewFrame();
