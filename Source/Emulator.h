#pragma once

#include "EmulatorCommon.h"

#include <shobjidl.h>

#include "CPU.h"
#include "GameTimer.h"
#include "resource.h"

class Emulator
{
public:
	Emulator(HINSTANCE hInstance);
	~Emulator();

	Emulator(const Emulator& rhs) = delete;
	Emulator& operator=(const Emulator& rhs) = delete;

public:
	static Emulator* GetEmulator();

	HINSTANCE GetAppInstance() const;

	HWND GetMainWindowHandle() const;

	void Initialise();

	int Run();

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool InitMainWindow();
	bool InitWindowStatusBar();

	void InitGraphics();

	void InitCpu();

	void OnResize();

	void OnStatusbarSize();
	void UpdateStatusBarText();

	void Update();
	void Draw();

	void LoadROM();

protected:
	static Emulator* m_Emulator;

private:
	CPU* m_Cpu = nullptr;

	GameTimer* m_GameTimer = nullptr;

	HINSTANCE m_AppInstance = nullptr;

	HWND m_MainWindowHandle = nullptr;

	bool m_bIsAppPaused = true;
	bool m_bIsProgramLoaded = false;

	bool m_bIsResizing = false;

	bool m_bIsMinimized = false;
	bool m_bIsMaximised = false;

	int m_ClientWindowWidth = 800;
	int m_ClientWindowHeight = 600;

	const std::wstring m_MainWindowCaption = L"CHIP-8 Emulator";

	const COMDLG_FILTERSPEC FileFilterSpec[3] =
	{
		{ L"CHIP-8 Program", L"*.ch8" },
		{ L"Binary Files",   L"*.bin" },
		{ L"All Files",      L"*.*" }
	};
};

