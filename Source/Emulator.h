#pragma once

#include "EmulatorCommon.h"

#include <shobjidl.h>

#include "CPU.h"
#include "GameTimer.h"
#include "Graphics.h"
#include "resource.h"

#define VK_0    0x30
#define VK_1    0x31
#define VK_2    0x32
#define VK_3    0x33
#define VK_4    0x34
#define VK_5    0x35
#define VK_6    0x36
#define VK_7    0x37
#define VK_8    0x38
#define VK_9    0x39
#define VK_A    0x41
#define VK_B    0x42
#define VK_C    0x43
#define VK_D    0x44
#define VK_E    0x45
#define VK_F    0x46

/*
* Main application class for running the emulator and managing the overall program state
*/
class Emulator
{
public:
	/// <summary>
	/// Creates a new instance of the Emulator program
	/// </summary>
	/// <param name="hInstance">HINSTANCE of the current application</param>
	Emulator(HINSTANCE hInstance);

	// Releases any in-use resources on program exit
	~Emulator();

	// Deleted functions. We have no need for these
	Emulator(const Emulator& rhs) = delete;
	Emulator& operator=(const Emulator& rhs) = delete;

public:
	/// <summary>
	/// The current Emulator class that's running
	/// </summary>
	/// <returns>Static instance of the current Emulator class</returns>
	static Emulator* GetEmulator();

	/// <summary>
	/// The current HINSTANCE of the running program
	/// </summary>
	/// <returns>HINSTANCE of the running program</returns>
	HINSTANCE GetAppInstance() const;

	/// <summary>
	/// The current HWND for the main application window
	/// </summary>
	/// <returns>HWND for the main application window</returns>
	HWND GetMainWindowHandle() const;

	/// <summary>
	/// Initialises the various sub-systems required by the emulator
	/// </summary>
	void Initialise();

	/// <summary>
	/// Main program loop. Handles Window events, CHIP-8 emulation, timers etc
	/// </summary>
	/// <returns></returns>
	int Run();

	/// </inheritdoc>
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	/// <summary>
	/// Creates the main application window
	/// </summary>
	/// <returns>True if the window was created successfully. If any errors/problems are encountered that prevents the window being created, then false is returned</returns>
	bool InitMainWindow();

	/// <summary>
	/// Creates the 'StatusBar' control which resides at the bottom of the main window
	/// </summary>
	/// <returns>True if the control was created successfully or false if any errors prevented the control being created</returns>
	bool InitWindowStatusBar();

	/// <summary>
	/// Initialises the renderer that will be used to display the emulators VRAM on-screen
	/// </summary>
	void InitGraphics();

	/// <summary>
	/// Initialises the CHIP-8 CPU emulator
	/// </summary>
	void InitCpu();

	/// <summary>
	/// Called every time the window is resized. Ensures size-sensitive objects (e.g. Graphics renderer) can react to the resize event.
	/// </summary>
	void OnResize();

	/// <summary>
	/// Called every time the window is resized. Resizes the 'StatusBar' at the bottom of the window accordingly
	/// </summary>
	void OnStatusbarSize();

	/// <summary>
	/// Sets the text displayed in the 'StatusBar' accordingly based on the current state of the program
	/// </summary>
	void UpdateStatusBarText();

	/// <summary>
	/// Updates the internal state of the emulator
	/// </summary>
	/// <param name="DeltaTime">Time elapsed (in seconds) since the last call to Update</param>
	void Update(float DeltaTime);

	/// <summary>
	/// Draws the emulator VRAM to screen
	/// </summary>
	void Draw();

	/// <summary>
	/// Decrements the CPU's 'Sound' and/or 'Delay' timers by 60 each second if they're currently greater than 0
	/// </summary>
	void UpdateTimers();

	/// <summary>
	/// Called when an end-user clicks on the 'File' > 'Load ROM' menu. Displays a 'File Browse Dialog' and attempts to load the user-selected file into the CPU's memory.
	/// </summary>
	void LoadROM();

protected:
	// Static instance of the current class
	static Emulator* m_Emulator;

private:
	// Pointer to the CPU instance that will be used for emulation
	CPU* m_Cpu = nullptr;

	// Game timer class used for handling timer-related emulation tasks
	GameTimer* m_GameTimer = nullptr;

	// The current application instance
	HINSTANCE m_AppInstance = nullptr;

	// Handle for the main application window
	HWND m_MainWindowHandle = nullptr;

	// If the app is currently paused (e.g. No ROM is loaded, window being moved/resized etc) then this will set to true. When true all rendering and loops will also be paused.
	bool m_bIsAppPaused = true;

	// True if a ROM has been loaded into the CHIP-8 memory. Sets that we're ready to begin emulation.
	bool m_bIsProgramLoaded = false;

	// True if the application window is currently being resized. If so we want to pause all rendering related tasks until the resize is complete.
	bool m_bIsResizing = false;

	// Current window state (e.g. Minimised or maximised). Allows the renderer to react accrodingly
	bool m_bIsMinimized = false;
	bool m_bIsMaximised = false;

	// Dimensions of the window. Allows the renderer to react accordingly.
	int m_ClientWindowWidth = 800;
	int m_ClientWindowHeight = 600;

	// Caption to display in the TitleBar of the main application window
	const std::wstring m_MainWindowCaption = L"CHIP-8 Emulator";

	// List of file types selectable on the 'Open File Dialog' when browsing to a ROM file on disk.
	const COMDLG_FILTERSPEC FileFilterSpec[3] =
	{
		{ L"CHIP-8 Program", L"*.ch8" },
		{ L"Binary Files",   L"*.bin" },
		{ L"All Files",      L"*.*" }
	};
};

