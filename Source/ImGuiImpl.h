#pragma once

#include <SDL.h>

#include <imgui.h>
#include <imgui_sdl.h>

#include "imgui_memory_editor.h"

#include "CPU.h"

/*
* Implementation for Dear ImGui integration.
* 
* Handles the setup, update and event handling for Dear ImGui behind the scenes
*/
class ImGuiImpl
{
public:
	/// <summary>
	/// Initialises Dear ImGui and its SDL counterpart, ready for drawing to take place.
	/// Also handles mapping ImGui keycodes to SDL keycodes.
	/// </summary>
	/// <param name="renderer">The renderer ImGui should use to draw itself to the screen</param>
	/// <param name="windowWidth">Width of the main application window</param>
	/// <param name="windowHeight">Height of the main application window</param>
	void Init(SDL_Renderer* renderer, int windowWidth, int windowHeight);

	/// <summary>
	/// Unloads the ImGui instance being used to draw to the screen and releases any resources it holds
	/// </summary>
	void Destroy();

	/// <summary>
	/// Handles events for Dear ImGui, such as window resizes or keyboard/mouse input events.
	/// </summary>
	/// <param name="sdlEvent">The event which occured that ImGui may need to react to</param>
	void HandleEvent(SDL_Event* sdlEvent);

	/// <summary>
	/// Updates ImGui with the current mouse state and time that has passed since the previous frame
	/// </summary>
	/// <param name="deltaTime">Time (in seconds) since 'Update()' was last called</param>
	void Update(float deltaTime);
};

