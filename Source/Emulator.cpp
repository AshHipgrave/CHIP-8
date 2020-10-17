#include "Emulator.h"

bool Emulator::Initialise()
{
	m_GameTimer = new GameTimer();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "ERROR: Failed to initialise SDL2: " << SDL_GetError() << std::endl;
		return false;
	}

	m_GameWindow = SDL_CreateWindow(k_WindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, k_WindowWidth, k_WindowHeight, SDL_WINDOW_SHOWN);

	if (m_GameWindow == nullptr)
	{
		std::cout << "ERROR: Failed to create main window: " << SDL_GetError() << std::endl;
		return false;
	}

	m_Renderer = SDL_CreateRenderer(m_GameWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (m_Renderer == nullptr)
	{
		std::cout << "ERROR: Failed to initialise renderer: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_SetRenderDrawColor(m_Renderer, 100, 149, 237, 255);

	m_RenderTexture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
	
	InitCpu();

	if (!LoadRom())
	{
		std::cout << "Failed to load to a ROM file" << std::endl;
		return false;
	}

	return true;
}

void Emulator::InitCpu()
{
	m_Cpu = new CPU();
	m_Cpu->Init();
}

void Emulator::Run()
{
	m_bIsRunning = true;

	m_GameTimer->Reset();

	while (m_bIsRunning)
	{
		m_GameTimer->Tick();

		HandleEvents();

		if (m_bIsRunning) // 'm_bIsRunning' may have been set to 'false' as part of 'HandleEvents'. We don't want to just blindly continue executing if SDL has been torn down
		{
			Update();

			m_Cpu->RunCycle();

			Draw();
		}
	}
}

bool Emulator::LoadRom()
{
	bool success = false;

	IFileOpenDialog* fileOpenDialog;

	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpenDialog))))
		{
			fileOpenDialog->SetFileTypes(3, k_FileFilterSpec);
			fileOpenDialog->SetTitle(L"Open ROM");

			// Get the window handle from SDL. Need to obtain the SDL_Version first as for some reason it's required :|
			SDL_SysWMinfo windowInfo;
			SDL_VERSION(&windowInfo.version);
			SDL_GetWindowWMInfo(m_GameWindow, &windowInfo);

			HWND windowHandle = windowInfo.info.win.window;

			if (SUCCEEDED(fileOpenDialog->Show(windowHandle)))
			{
				IShellItem* item;

				if (SUCCEEDED(fileOpenDialog->GetResult(&item)))
				{
					PWSTR filePath;

					if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath)))
					{
						success = m_Cpu->LoadProgram(filePath);
					}
					item->Release();
				}
			}
			fileOpenDialog->Release();
		}
		CoUninitialize();
	}
	return success;
}

void Emulator::HandleEvents()
{
	SDL_Event sdlEvent;

	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
			case SDL_QUIT:
			{
				Stop();
				break;
			}
		}
	}
}

void Emulator::Update()
{
	m_KeyStates = const_cast<Uint8*>(SDL_GetKeyboardState(0));

	for (Uint8 i = 0; i < 16; i++)
	{
		if (m_KeyStates[k_KeyCodes[i]] == 1)
		{
			m_Cpu->SetKeyState(i);
		}
		else
		{
			m_Cpu->ClearKeyState(i);
		}
	}
}

void Emulator::Draw()
{
	uint8_t* vram = m_Cpu->GetState()->VideoMemory;
	
	uint32_t pixels[2048];

	for (int i = 0; i < 2048; ++i)
	{
		uint8_t pixel = vram[i];
		pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
	}

	// Update SDL texture
	SDL_UpdateTexture(m_RenderTexture, NULL, pixels, 64 * sizeof(Uint32));

	// Clear screen and render
	SDL_RenderClear(m_Renderer);
	SDL_RenderCopy(m_Renderer, m_RenderTexture, NULL, NULL);
	SDL_RenderPresent(m_Renderer);
}

void Emulator::UpdateTimers()
{
	static float timeElapsed = 0.0f;

	if (m_GameTimer->TotalTime() - timeElapsed >= 1.0f)
	{
		const ChipState* cpuState = m_Cpu->GetState();

		if (cpuState->Delay > 0)
		{
			uint8_t newValue = cpuState->Delay - ((cpuState->Delay < 60 ) ? cpuState->Delay : 60);

			m_Cpu->SetDelayRegister(newValue);
		}

		if (cpuState->Sound > 0)
		{
			uint8_t newValue = cpuState->Sound - ((cpuState->Sound < 60) ? cpuState->Sound : 60);

			m_Cpu->SetSoundRegister(newValue);
		}

		timeElapsed += 1.0f;
	}
}

void Emulator::Stop()
{
	m_bIsRunning = false;

	if (m_RenderTexture != nullptr)
		SDL_DestroyTexture(m_RenderTexture);

	if (m_Renderer != nullptr)
		SDL_DestroyRenderer(m_Renderer);

	if (m_GameWindow != nullptr)
		SDL_DestroyWindow(m_GameWindow);

	SDL_Quit();
}