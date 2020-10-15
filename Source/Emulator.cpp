#include "Emulator.h"

Emulator* Emulator::m_Emulator = nullptr;

Emulator::Emulator(HINSTANCE hInstance)
	: m_AppInstance(hInstance)
{
	m_Emulator = this;
}

Emulator::~Emulator()
{

}

Emulator* Emulator::GetEmulator()
{
	return Emulator::m_Emulator;
}

HINSTANCE Emulator::GetAppInstance() const
{
	return m_AppInstance;
}

HWND Emulator::GetMainWindowHandle() const
{
	return m_MainWindowHandle;
}

void Emulator::Initialise()
{
	InitMainWindow();
	InitGraphics();
	InitCpu();
}

int Emulator::Run()
{
	MSG msg = { 0 };

	m_GameTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			m_GameTimer.Tick();

			if (!m_bIsAppPaused)
			{
				Update();
				Draw();
			}
			else
			{
				::Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void Emulator::Update()
{
}

void Emulator::Draw()
{
}

void Emulator::LoadROM()
{
	IFileOpenDialog* fileOpenDialog;

	::ThrowIfFailed(
		CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpenDialog)));

	fileOpenDialog->SetFileTypes(3, FileFilterSpec);

	if (SUCCEEDED(fileOpenDialog->Show(m_MainWindowHandle)))
	{
		IShellItem* item;

		if (SUCCEEDED(fileOpenDialog->GetResult(&item)))
		{
			PWSTR filePath;

			if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath)))
			{
				m_Cpu->LoadProgram(filePath);
			}

			item->Release();
		}
	}

	fileOpenDialog->Release();
}

void Emulator::OnResize()
{

}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Emulator::GetEmulator()->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT Emulator::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case ID_FILE_LOADROM:
					LoadROM();
					return 0;
				case ID_DEBUG_SHOWDEBUGVIEW:
					// Show ImGui w/ register states etc.
					break;
			}
			break;
		}
		case WM_ACTIVATE:
		{
			if (LOWORD(wParam) == WA_INACTIVE)
			{
				m_bIsAppPaused = true;
				m_GameTimer.Stop();
			}
			else
			{
				m_bIsAppPaused = false;
				m_GameTimer.Start();
			}
			return 0;
		}
		case WM_SIZE:
		{
			m_ClientWindowWidth = LOWORD(lParam);
			m_ClientWindowHeight = HIWORD(lParam);

			if (wParam == SIZE_MINIMIZED)
			{
				m_bIsAppPaused = true;
				m_bIsMinimized = true;

				m_bIsMaximised = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_bIsAppPaused = false;
				m_bIsMinimized = false;

				m_bIsMaximised = true;
			}
			else if (wParam == SIZE_RESTORED)
			{
				if (m_bIsMinimized)
				{
					m_bIsAppPaused = false;
					m_bIsMinimized = false;

					OnResize();
				}
				else if (m_bIsMaximised)
				{
					m_bIsAppPaused = false;
					m_bIsMaximised = false;

					OnResize();
				}
				else if (m_bIsResizing)
				{
					// Do Nothing. Wait for the user to finish resizing the window, at which point WM_EXITSIZEMOVE is sent and we can correctly handle the new Window size.
					// Whilst I could combine this with the below to be: 'if (!m_bIsReiszing) { OnResize(); }' It's slightly more readable to know we're deliberately not doing anything (Also looks odd to call 'OnResize()' right after checking '!Resizing').
				}
				else
				{
					OnResize();
				}
			}
			return 0;
		}
		case WM_ENTERSIZEMOVE:
		{
			m_bIsAppPaused = true;
			m_bIsResizing = true;

			m_GameTimer.Stop();

			return 0;
		}
		case WM_EXITSIZEMOVE:
		{
			m_bIsAppPaused = false;
			m_bIsResizing = false;

			//m_GameTimer.Start();

			OnResize();

			return 0;
		}
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			return 0;
		}
		case WM_MENUCHAR:
		{
			return MAKELRESULT(0, MNC_CLOSE);
		}
		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;

			return 0;
		}
		/*
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}
		*/
		case WM_KEYUP:
		{
			if (wParam == VK_ESCAPE)
			{
				::PostQuitMessage(0);
			}

			return 0;
		}
	}

	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool Emulator::InitMainWindow()
{
	WNDCLASS windowClass = { 0 };
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = MainWndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = m_AppInstance;
	windowClass.hIcon = ::LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = ::LoadCursor(0, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)::GetStockObject(NULL_BRUSH);
	windowClass.lpszMenuName = TEXT("IDR_MENU_MAIN"); //0;
	windowClass.lpszClassName = L"CHIP8Window";

	if (!::RegisterClass(&windowClass))
	{
		::MessageBox(0, L"Failed to register WindowClass", L"Error creating MainWindow", MB_ICONERROR);
		return false;
	}

	RECT windowRect = { 0, 0, m_ClientWindowWidth, m_ClientWindowHeight };

	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;

	m_MainWindowHandle = ::CreateWindow(L"CHIP8Window", m_MainWindowCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppInstance, 0);

	if (!m_MainWindowHandle)
	{
		::MessageBox(0, L"Failed to create main window", L"Error creating MainWindow", MB_ICONERROR);
		return false;
	}

	::ShowWindow(m_MainWindowHandle, SW_SHOW);
	::UpdateWindow(m_MainWindowHandle);
}

void Emulator::InitGraphics()
{

}

void Emulator::InitCpu()
{
	m_Cpu = new CPU();
	m_Cpu->Init();
}