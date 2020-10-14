#pragma once

#include "EmulatorCommon.h"

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
	void InitGraphics();

	void InitCpu();

	void OnResize();

	void Update();
	void Draw();

protected:
	static Emulator* m_Emulator;

private:
	CPU* m_Cpu = nullptr;

	HINSTANCE m_AppInstance = nullptr;

	HWND m_MainWindowHandle = nullptr;

	bool m_bIsAppPaused = false;

	bool m_bIsResizing = false;

	bool m_bIsMinimized = false;
	bool m_bIsMaximised = false;

	int m_ClientWindowWidth = 800;
	int m_ClientWindowHeight = 600;

	GameTimer m_GameTimer;

	std::wstring m_MainWindowCaption = L"CHIP-8 Emulator";
};

