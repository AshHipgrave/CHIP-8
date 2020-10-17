#pragma once

#include <SDL.h>

#include <imgui.h>
#include <imgui_sdl.h>

#include "imgui_memory_editor.h"

#include "CPU.h"

class ImGuiImpl
{
public:
	void Init(SDL_Renderer* renderer, int windowWidth, int windowHeight);
	void Destroy();

	void HandleEvent(SDL_Event* sdlEvent);

	void Update();
	void Draw(CPU* cpuInstance);

	bool IsInitialised();

private:
	void DrawMainMenu();
	void DrawDebugOverlay(ChipState* state);

private:
	static const char* GetClipboardText(void*);

	static void SetClipboardText(void*, const char* text);

private:
	bool m_bShowVRamView = false;

	bool m_bShowStackView = false;

	bool m_bShowSystemMemoryView = false;

	bool m_bShowRegistersWindow = false;

	bool m_bShowDebugOverlay = false;

private:
	bool m_bIsInitialised = false;

	bool m_MouseButtonPressed[3] = { false, false, false };

	SDL_Cursor* m_MouseCursors[ImGuiMouseCursor_COUNT] = {};

	MemoryEditor* m_VRamWindow = nullptr;
	MemoryEditor* m_StackMemoryWindow = nullptr;
	MemoryEditor* m_SystemMemoryWindow = nullptr;
};

