#pragma once

#include "EmulatorCommon.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <shobjidl.h>

#include "CPU.h"
#include "GameTimer.h"
#include "ImGuiImpl.h"
#include "imgui_memory_editor.h"

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
	/// Initialises Dear ImGui integration
	/// </summary>
	void InitImGui();

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
	/// Clears the display and prepares to draw a new frame
	/// </summary>
	void Clear();

	/// <summary>
	/// Draws the emulator VRAM and Dear ImGui UI to screen
	/// </summary>
	void Draw();

	/// <summary>
	/// Pushes the current back buffer to screen to be drawn
	/// </summary>
	void Present();

	/// <summary>
	/// Decrements the CPU's 'Sound' and/or 'Delay' timers each second if they're currently greater than 0
	/// </summary>
	void UpdateTimers();

private:
	/// <summary>
	/// Draws the ImGui menu bar at the top of the screen
	/// </summary>
	void DrawMainMenu();

	/// <summary>
	/// Draws the ImGui debug overlay showing the contents of each CPU register
	/// </summary>
	void DrawDebugOverlay();

private:
	// Set to true if the emulator is currently running (Not including the CPU)
	bool m_bIsRunning = false;

	// Set to true if the CPU is paused and not executing any more instructions
	bool m_bIsCpuPaused = true;

	// Set to true if a ROM has been loaded into the CPU's memory ready for execution. False if no ROM has been loaded.
	bool m_bIsProgramLoaded = false;

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

	// ImGui implementation. Handles key presses and state for the ImGui integration
	ImGuiImpl* m_ImGuiContext = nullptr;

private:
	/*ImGui Memory Viewers*/

	// Memory viewer for the CPU's VRAM
	MemoryEditor* m_VRamWindow = nullptr;

	// Memory viewer for the CPU's Stack
	MemoryEditor* m_StackMemoryWindow = nullptr;

	// Memory viewer for the CPU's full memory view
	MemoryEditor* m_SystemMemoryWindow = nullptr;

	/* ImGui State variables */

	// Set to true if the ImGui VRAM memory viewer should be displayed on-screen
	bool m_bShowVRamView = false;

	// Set to true if the ImGui Stack memory viewer should be displayed on-screen
	bool m_bShowStackView = false;

	// Set to true if the ImGui CPU memory viewer should be displayed on-screen
	bool m_bShowSystemMemoryView = false;

	// Set to true if the ImGui registers overlay should be displayed on-screen
	bool m_bShowDebugOverlay = false;

	// If set to true the CPU will execute a single instruction and then pause again
	bool m_bExecuteSingleInstruction = false;

private:

	/* Constants */

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