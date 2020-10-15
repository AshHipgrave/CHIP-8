#pragma once

#include <cstdint>

/*
* Graphical sprites native to the Chip-8 that can be displayed on-screen.
*/
class Sprites
{
public:
	// Array of all 16 characters native to the CHIP-8. Each character is made up of 5 bytes.
	static const uint8_t Font[];

	// Total size of the 'Font' array (5 bytes per sprite, 16 sprites in total)
	static const uint8_t FONT_SIZE = 5 * 16;

	// Starting address of the Fonts in memory
	static const uint16_t FONT_START = 0;
};