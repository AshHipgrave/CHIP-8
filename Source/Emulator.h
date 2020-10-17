#pragma once

#include "EmulatorCommon.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <shobjidl.h>

#include "CPU.h"
#include "GameTimer.h"
#include "ImGuiImpl.h"

/*
* Main application class for running the emulator and managing the overall program state
*/
class Emulator
{
public:
	/// <summary>
	/// Initialises the various sub-systems required by the emulator
	/// </summary>
	bool Initialise();

	/// <summary>
	/// Main program loop. Handles Window events, CHIP-8 emulation, timers etc
	/// </summary>
	/// <returns></returns>
	void Run();

	/// <summary>
	/// Stops the emulator and releases and in-use resources
	/// </summary>
	void Stop();

private:
	/// <summary>
	/// Initialises the CHIP-8 CPU emulator and loads a program
	/// </summary>
	void InitCpu();

	/// <summary>
	/// Displays a 'File Browse Dialog' for the end-user to select a ROM to load from disk.
	/// </summary>
	/// <returns>True if the user selected a ROM and it was loaded successfully. Otherwsie false.</returns>
	bool LoadRom();

	/// <summary>
	/// Handles any pending SDL or Windows window events
	/// </summary>
	void HandleEvents();

	/// <summary>
	/// Checks for key press events and if found 
	/// </summary>
	void Update();

	/// <summary>
	/// Draws the emulator VRAM to screen
	/// </summary>
	void Draw();

	/// <summary>
	/// Decrements the CPU's 'Sound' and/or 'Delay' timers by 60 each second if they're currently greater than 0
	/// </summary>
	void UpdateTimers();

private:
	// Set to true if the CPU is currently running a program or false if no program is currently executing
	bool m_bIsRunning = false;

	// Buffer for uploading VRAM to the GPU for rendering
	uint32_t m_PixelBuffer[2048];

	// State of each keyboard key (i.e. Is it pressed or not)
	Uint8* m_KeyStates = nullptr;

	// Main application window for the emulator
	SDL_Window* m_GameWindow = nullptr;

	// The renderer that will draw the CHIP-8 VRAM to the screen
	SDL_Renderer* m_Renderer = nullptr;

	// Texture the CHIP-8 display will be drawn to before being presented to the display
	SDL_Texture* m_RenderTexture = nullptr;

	// Pointer to the CPU instance that will be used for emulation
	CPU* m_Cpu = nullptr;

	// Game timer class used for handling timer-related emulation tasks
	GameTimer* m_GameTimer = nullptr;

	ImGuiImpl* m_ImGuiContext = nullptr;

private:
	// Dimensions the main application window will be created at
	int k_WindowWidth = 1440;
	int k_WindowHeight = 900;

	// Title displayed for the main appliaction window
	const char* k_WindowTitle = "CHIP-8 Emulator";

	// List of file types selectable on the 'Open File Dialog' when browsing to a ROM file on disk.
	const COMDLG_FILTERSPEC k_FileFilterSpec[3] =
	{
		{ L"CHIP-8 Program", L"*.ch8" },
		{ L"Binary Files",   L"*.bin" },
		{ L"All Files",      L"*.*" }
	};

	// Map of SDL2 keycodes for the various keyboard keys the CHIP-8 can handle/react to
	const Uint8 k_KeyCodes[16] =
	{
		SDL_SCANCODE_0,
		SDL_SCANCODE_1,
		SDL_SCANCODE_2,
		SDL_SCANCODE_3,
		SDL_SCANCODE_4,
		SDL_SCANCODE_5,
		SDL_SCANCODE_6,
		SDL_SCANCODE_7,
		SDL_SCANCODE_8,
		SDL_SCANCODE_9,
		SDL_SCANCODE_A,
		SDL_SCANCODE_B,
		SDL_SCANCODE_C,
		SDL_SCANCODE_D,
		SDL_SCANCODE_E,
		SDL_SCANCODE_F
	};
};