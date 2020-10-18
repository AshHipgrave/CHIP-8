#include "Emulator.h"

bool Emulator::Initialise()
{
	m_GameTimer = new GameTimer();

	if (!InitSDL())
		return false;
	
	InitCpu();
	InitImGui();

	return true;
}

bool Emulator::InitSDL()
{
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
	SDL_RenderSetLogicalSize(m_Renderer, k_WindowWidth, k_WindowHeight);

	if (m_Renderer == nullptr)
	{
		std::cout << "ERROR: Failed to initialise renderer: " << SDL_GetError() << std::endl;
		return false;
	}

	SDL_SetRenderDrawColor(m_Renderer, 100, 149, 237, 255);

	m_RenderTexture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	if (m_RenderTexture == nullptr)
	{
		std::cout << "ERROR: Failed to create render texture: " << SDL_GetError() << std::endl;
		return false;
	}
}

void Emulator::InitCpu()
{
	m_Cpu = new CPU();
	m_Cpu->Init();
}

void Emulator::InitImGui()
{
	m_ImGuiContext = new ImGuiImpl();
	m_ImGuiContext->Init(m_Renderer, k_WindowWidth, k_WindowHeight);

	m_VRamWindow = new MemoryEditor();
	m_StackMemoryWindow = new MemoryEditor();
	m_SystemMemoryWindow = new MemoryEditor();
}

void Emulator::Run()
{
	m_bIsRunning = true;

	m_GameTimer->Reset();

	while (m_bIsRunning)
	{
		m_GameTimer->Tick();

		if (m_bIsProgramLoaded && !m_bIsPaused)
		{
			m_Cpu->RunCycle();

			if (m_bExecuteSingleInstruction)
			{
				m_bIsPaused = true;
				m_bExecuteSingleInstruction = false;
			}
		}

		HandleEvents();
		Update();

		Clear();
		Draw();
		Present();
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
						m_bIsProgramLoaded = success;
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

	m_ImGuiContext->HandleEvent(&sdlEvent);
}

void Emulator::Update()
{
	m_ImGuiContext->Update(m_GameTimer->DeltaTime());

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

	UpdateTimers();
}

void Emulator::Clear()
{
	SDL_RenderClear(m_Renderer);

	ImGui::NewFrame();
}

void Emulator::Draw()
{
	for (int i = 0; i < 2048; i++)
	{
		uint8_t pixel = m_Cpu->GetState()->VideoMemory[i];

		m_PixelBuffer[i] = (0x00FFFFFF * pixel) | 0xFF000000;
	}

	SDL_UpdateTexture(m_RenderTexture, NULL, m_PixelBuffer, 64 * sizeof(uint32_t));
	SDL_RenderCopy(m_Renderer, m_RenderTexture, NULL, NULL);

	DrawMainMenu();

	if (m_bShowDebugOverlay)
		DrawDebugOverlay();

	if (m_bShowVRamView)
		m_VRamWindow->DrawWindow("VRAM View", (void *)&m_Cpu->GetState()->VideoMemory, 2048);

	if (m_bShowSystemMemoryView)
		m_SystemMemoryWindow->DrawWindow("System Memory", (void*)&m_Cpu->GetState()->Memory, 4096);

	if (m_bShowStackView)
		m_StackMemoryWindow->DrawWindow("Stack",(void *)& m_Cpu->GetState()->Stack, 16);

	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
}

void Emulator::Present()
{
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

void Emulator::DrawMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load ROM"))
			{
				LoadRom();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Run"))
		{
			if (ImGui::MenuItem("Execute ROM", NULL, false, (m_bIsProgramLoaded && m_bIsPaused)))
			{
				m_bIsPaused = false;
			}

			if (ImGui::MenuItem("Execute Single Instruction", NULL, false, (m_bIsProgramLoaded && m_bIsPaused)))
			{
				m_bIsPaused = false;
				m_bExecuteSingleInstruction = !m_bExecuteSingleInstruction;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Show Debug Overlay",       NULL,  &m_bShowDebugOverlay);
			ImGui::MenuItem("Show Stack",               NULL,  &m_bShowStackView);
			ImGui::MenuItem("Show Full System Memory",  NULL,  &m_bShowSystemMemoryView);
			ImGui::MenuItem("Show VRAM",                NULL,  &m_bShowVRamView);

			ImGui::EndMenu();
		}
	}

	ImGui::EndMainMenuBar();
}

void Emulator::DrawDebugOverlay()
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

		ImU16 sp = m_Cpu->GetState()->SP;
		ImGui::Text("Stack Pointer:   ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, sp);

		ImU16 pc = m_Cpu->GetState()->PC;
		ImGui::Text("Program Counter: ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, pc);

		ImU16 iReg = m_Cpu->GetState()->I;
		ImGui::Text("I Register:      ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, iReg);

		ImGui::Separator();

		ImU8 delay = m_Cpu->GetState()->Delay;
		ImGui::Text("Delay Register:  ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, delay);

		ImU8 sound = m_Cpu->GetState()->Sound;
		ImGui::Text("Sound Register:  ");
		ImGui::SameLine();
		ImGui::Text(format_byte_space, sound);

		ImGui::Separator();

		for (int i = 0; i < 16; i++)
		{
			const char* padding = (i < 10) ? "............" : "...........";

			ImU8 vReg = m_Cpu->GetState()->V[i];
			ImGui::Text("V[%d] %s", i, padding);
			ImGui::SameLine();
			ImGui::Text(format_byte_space, vReg);
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0))  corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1))  corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2))  corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3))  corner = 3;

			if (&m_bShowDebugOverlay && ImGui::MenuItem("Close")) m_bShowDebugOverlay = false;

			ImGui::EndPopup();
		}
	}
	ImGui::End();
}

void Emulator::Stop()
{
	m_bIsRunning = false;

	m_Cpu->Stop();

	if (m_RenderTexture != nullptr)
		SDL_DestroyTexture(m_RenderTexture);

	if (m_Renderer != nullptr)
		SDL_DestroyRenderer(m_Renderer);

	if (m_GameWindow != nullptr)
		SDL_DestroyWindow(m_GameWindow);

	SDL_Quit();
}