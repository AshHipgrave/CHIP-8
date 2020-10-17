#include "ImGuiImpl.h"

void ImGuiImpl::Init(SDL_Renderer* renderer, int windowWidth, int windowHeight)
{
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, windowWidth, windowHeight);

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "imgui_impl_sdl";

	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
	io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

io.SetClipboardTextFn = SetClipboardText;
io.GetClipboardTextFn = GetClipboardText;
io.ClipboardUserData = NULL;

m_MouseCursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
m_MouseCursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
m_MouseCursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
m_MouseCursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
m_MouseCursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
m_MouseCursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
m_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
m_MouseCursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
m_MouseCursors[ImGuiMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

m_VRamWindow = new MemoryEditor();
m_StackMemoryWindow = new MemoryEditor();
m_SystemMemoryWindow = new MemoryEditor();

m_bIsInitialised = true;
}

void ImGuiImpl::Destroy()
{
	m_bIsInitialised = false;

	ImGuiSDL::Deinitialize();
	ImGui::DestroyContext();
}

void ImGuiImpl::HandleEvent(SDL_Event* sdlEvent)
{
	ImGuiIO& io = ImGui::GetIO();

	if (sdlEvent->type == SDL_WINDOWEVENT)
	{
		switch (sdlEvent->window.event)
		{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				ImGuiIO& io = ImGui::GetIO();

				io.DisplaySize.x = static_cast<float>(sdlEvent->window.data1);
				io.DisplaySize.y = static_cast<float>(sdlEvent->window.data2);

				break;
		}
	}
	else if (sdlEvent->type == SDL_MOUSEWHEEL)
	{
		io.MouseWheel = static_cast<float>(sdlEvent->wheel.y);
	}
	else if (sdlEvent->type == SDL_MOUSEBUTTONDOWN)
	{
		if (sdlEvent->button.button == SDL_BUTTON_LEFT) m_MouseButtonPressed[0] = true;
		if (sdlEvent->button.button == SDL_BUTTON_RIGHT) m_MouseButtonPressed[1] = true;
		if (sdlEvent->button.button == SDL_BUTTON_MIDDLE) m_MouseButtonPressed[2] = true;
	}
	else if (sdlEvent->type == SDL_TEXTINPUT)
	{
		io.AddInputCharactersUTF8(sdlEvent->text.text);
	}
	else if (sdlEvent->type == SDL_KEYDOWN || sdlEvent->type == SDL_KEYUP)
	{
		int keyCode = sdlEvent->key.keysym.scancode;

		io.KeysDown[keyCode] = (sdlEvent->type == SDL_KEYDOWN);

		io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
	}
}

void ImGuiImpl::Update()
{
	int mouseX, mouseY;

	const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

	ImGuiIO& io = ImGui::GetIO();

	io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));

	io.MouseDown[0] = m_MouseButtonPressed[0] || buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
	io.MouseDown[1] = m_MouseButtonPressed[0] || buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
	io.MouseDown[2] = m_MouseButtonPressed[0] || buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);

	m_MouseButtonPressed[0] = m_MouseButtonPressed[1] = m_MouseButtonPressed[2] = false;
}

void ImGuiImpl::Draw(CPU* cpuInstance)
{
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();

	DrawMainMenu();

	if (m_bShowDebugOverlay)
	{
		DrawDebugOverlay(cpuInstance->m_CpuState);
	}

	if (m_bShowVRamView)
	{
		m_VRamWindow->DrawWindow("VRAM View", cpuInstance->m_CpuState->VideoMemory, 2048);
	}

	if (m_bShowSystemMemoryView)
	{
		m_SystemMemoryWindow->DrawWindow("System Memory", cpuInstance->m_CpuState->Memory, 4096);
	}

	if (m_bShowStackView)
	{
		m_StackMemoryWindow->DrawWindow("Stack", &cpuInstance->m_CpuState->Memory[cpuInstance->m_CpuState->SP], 512);
	}

	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
}

void ImGuiImpl::DrawMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Show Debug Overlay", "", &m_bShowDebugOverlay);
			ImGui::MenuItem("Show Registers Window", "", &m_bShowRegistersWindow);
			ImGui::MenuItem("Show Stack", "", &m_bShowStackView);
			ImGui::MenuItem("Show Full System Memory", "", &m_bShowSystemMemoryView);
			ImGui::MenuItem("Show VRAM", "", &m_bShowVRamView);

			ImGui::EndMenu();
		}
	}

	ImGui::EndMainMenuBar();
}

void ImGuiImpl::DrawDebugOverlay(ChipState* state)
{
	const float DISTANCE = 10.0f;
	static int corner = 2;

	ImGuiIO& io = ImGui::GetIO();

	if (corner != -1)
	{
		ImVec2 windowPosition = ImVec2(
			(corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, 
			(corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);

		// https://media.giphy.com/media/2OP9jbHFlFPW/giphy.gif
		ImVec2 windowPostionPivot = ImVec2(
			(corner & 1) ? 1.0f : 0.0f, 
			(corner & 2) ? 1.0f : 0.0f);

		ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always, windowPostionPivot);
	}

	ImGui::SetNextWindowBgAlpha(0.35f);

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	if (corner == -1)
	{
		windowFlags |= ImGuiWindowFlags_NoMove;
	}

	if (ImGui::Begin("Debug View", &m_bShowDebugOverlay, windowFlags))
	{
		const char* format_byte_space = "%02X ";

		ImU16 sp = state->SP;
		ImGui::Text("Stack Pointer: ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, sp);

		ImU16 pc = state->PC;
		ImGui::Text("Program Counter: ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, pc);

		ImU16 iReg = state->I;
		ImGui::Text("I Register: ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, iReg);

		ImGui::Separator();

		ImU8 delay = state->Delay;
		ImGui::Text("Delay Register: ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, delay);

		ImU8 sound = state->Sound;
		ImGui::Text("Sound Register: ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, sound);

		ImGui::Separator();

		for (int i = 0; i < 16; i++)
		{
			ImU8 vReg = state->V[i];
			ImGui::Text("V[%d]", i);
			ImGui::SameLine();
			ImGui::Text(format_byte_space, vReg);
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom",       NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left",     NULL, corner == 0))  corner = 0;
			if (ImGui::MenuItem("Top-right",    NULL, corner == 1))  corner = 1;
			if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2))  corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3))  corner = 3;

			if (&m_bShowDebugOverlay && ImGui::MenuItem("Close")) m_bShowDebugOverlay = false;

			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

bool ImGuiImpl::IsInitialised()
{
	return m_bIsInitialised;
}

const char* ImGuiImpl::GetClipboardText(void*)
{
	return SDL_GetClipboardText();
}

void ImGuiImpl::SetClipboardText(void*, const char* text)
{
	SDL_SetClipboardText(text);
}
