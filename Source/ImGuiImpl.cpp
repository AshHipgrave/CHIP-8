#include "ImGuiImpl.h"

void ImGuiImpl::Init(SDL_Renderer* renderer, int windowWidth, int windowHeight)
{
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, windowWidth, windowHeight);

	ImGuiIO& io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_A] = SDLK_a;
	io.KeyMap[ImGuiKey_C] = SDLK_c;

	io.KeyMap[ImGuiKey_V] = SDLK_v;
	io.KeyMap[ImGuiKey_X] = SDLK_x;

	io.KeyMap[ImGuiKey_Y] = SDLK_y;
	io.KeyMap[ImGuiKey_Z] = SDLK_z;

	io.KeyMap[ImGuiKey_Tab] = SDL_GetScancodeFromKey(SDLK_TAB);
	io.KeyMap[ImGuiKey_End] = SDL_GetScancodeFromKey(SDLK_END);

	io.KeyMap[ImGuiKey_Home] = SDL_GetScancodeFromKey(SDLK_HOME);
	io.KeyMap[ImGuiKey_Enter] = SDL_GetScancodeFromKey(SDLK_RETURN);

	io.KeyMap[ImGuiKey_Delete] = SDL_GetScancodeFromKey(SDLK_DELETE);
	io.KeyMap[ImGuiKey_Escape] = SDL_GetScancodeFromKey(SDLK_ESCAPE);

	io.KeyMap[ImGuiKey_UpArrow] = SDL_GetScancodeFromKey(SDLK_UP);
	io.KeyMap[ImGuiKey_DownArrow] = SDL_GetScancodeFromKey(SDLK_DOWN);

	io.KeyMap[ImGuiKey_LeftArrow] =  SDL_GetScancodeFromKey(SDLK_LEFT);
	io.KeyMap[ImGuiKey_RightArrow] = SDL_GetScancodeFromKey(SDLK_RIGHT);

	io.KeyMap[ImGuiKey_Backspace] = SDL_GetScancodeFromKey(SDLK_BACKSPACE);
}

void ImGuiImpl::Destroy()
{
	ImGuiSDL::Deinitialize();

	ImGui::DestroyContext();
}

void ImGuiImpl::HandleEvent(SDL_Event* sdlEvent)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel = 0.0f;

	switch (sdlEvent->type)
	{
		case SDL_WINDOWEVENT:
		{
			if (sdlEvent->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				io.DisplaySize.x = static_cast<float>(sdlEvent->window.data1);
				io.DisplaySize.y = static_cast<float>(sdlEvent->window.data2);
			}
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			io.MouseWheel = static_cast<float>(sdlEvent->wheel.y);
			break;
		}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			SDL_Scancode keycode = sdlEvent->key.keysym.scancode;

			if (keycode >= 0 && keycode < 512)
			{
				bool bIsPressed = sdlEvent->key.type == SDL_KEYDOWN;

				io.KeysDown[keycode] = bIsPressed;
			}
			break;
		}
		case SDL_TEXTINPUT:
		{
			char* text = sdlEvent->text.text;

			for (int i = 0; i < strnlen(text, 32); i++)
			{
				uint32_t keycode = text[i];

				if (keycode >= 32 && keycode <= 255)
				{
					io.AddInputCharacter((char)keycode);
				}
			}
			break;
		}
	}
}

void ImGuiImpl::Update(float deltaTime)
{
	int mouseX, mouseY;

	const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = deltaTime;

	io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));

	io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
	io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
	io.MouseDown[2] = buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
}