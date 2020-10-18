#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "EmulatorCommon.h"

#include <errno.h>

#include "Sprites.h"

/**
 * Represents the internal state of the CPU (Stack pointer, registers, memory etc)
 */
struct ChipState
{
	// 16 8-bit general purpose registers (Referred to by OpCodes as Vx, where X = the specific register the OpCode references).
	uint8_t V[16] = { 0 };

	// I register - Usually stores memory addresses, so generally only the lowest 12-bits are used
	uint16_t I;

	// Program Counter - The current address in memory being executed
	uint16_t PC;

	// Stack Pointer - Points to the top-most level of the Stack
	uint16_t SP;

	uint16_t Stack[16];

	// Delay Register
	uint8_t Delay;

	// Sound Register
	uint8_t Sound;

	// Program memory of the loaded ROM
	uint8_t Memory[4096];

	// Video RAM for what is currently being drawn on-screen
	uint8_t VideoMemory[2048];

	// Keyboard key states (0 = Up | 1 = Down). Only 16 keys are available on the CHIP-8.
	uint8_t KeyState[16] = { 0 };

	// Keyboard key states from the previous execution cycle (Quick-and-dirty way of checking if a key has been pressed when we're blocked waiting for key presses)
	uint8_t PreviousKeyState[16] = { 0 };
	
	// Set to true if the CPU is paused waiting for a key to be pressed. Otherwise set to false.
	bool bIsWaitingForKeyPress = false;

	// If set to true the CPU won't execute any more instructions
	bool bIsStopped = true;
};

/**
 * CHIP-8 Interpreter / Emulated CPU
 */
class CPU
{
private:
	// Pointer to the current execution state of the CPU
	ChipState* m_CpuState = nullptr;

public:
	/// <summary>
	/// Initialises the CPU and sets the initial state. Must be called before trying to load a program.
	/// </summary>
	void Init();

	/// <summary>
	/// Stops the CPU executing any more instructions
	/// </summary>
	void Stop();

	/// <summary>
	/// Loads a ROM into the CPU's memory at address 0x200. CPU must be initialised before calling this function.
	/// </summary>
	/// <param name="FilePath">Path to the ROM file on disk that will be loaded into CPU memory</param>
	/// <returns>True if the program was loaded successfully. Otherwise false</returns>
	bool LoadProgram(const wchar_t* FilePath);

	/// <summary>
	/// Runs a single CPU cycle, emulating the current instruction being pointed to by the program counter
	/// </summary>
	void RunCycle();

	/// <summary>
	/// Sets the state of the specified key as Pressed
	/// </summary>
	/// <param name="keycode">Key being pressed</param>
	void SetKeyState(uint8_t keycode);

	/// <summary>
	/// Clears the state of the specified key so that it's not being pressed.
	/// </summary>
	/// <param name="keycode">Key being released</param>
	void ClearKeyState(uint8_t keycode);

	/// <summary>
	/// Sets the Delay register to the specified value.
	/// </summary>
	/// <param name="value">The new value to set the Delay register to</param>
	void SetDelayRegister(uint8_t value);

	/// <summary>
	/// Sets the Sound register to the specified value.
	/// </summary>
	/// <param name="value">The new value to set the Sound register to</param>
	void SetSoundRegister(uint8_t value);

	/// <summary>
	/// Gets the current state of the CPU
	/// </summary>
	/// <returns>Current state of the CPU</returns>
	const ChipState* GetState() const;

private:
	/// <summary>
	/// 0x0nnn instructions:
	///		0x0nnn = Jump to a machine code routine at address 'nnn' (Only implemented on original CHIP-8 PC's. Ignored for emulators and modern interpreters).
	///		0x00E0 = Clear the screen
	///		0x00EE = Return from a subroutine
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op0(uint16_t opcode);

	/// <summary>
	/// Jump to location instructions. Encoded as 0x1nnn where 'nnn' is the address to jump to.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op1(uint16_t opcode);

	/// <summary>
	/// Call address instructions. Encoded as 0x2nnn where 'nnn' is the address to call.
	/// Stack Pointer is incremented and the Program Counter saved to the stack before calling.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op2(uint16_t opcode);

	/// <summary>
	/// Skips the next instruction if the register Vx is equal to specified value. Encoded as 0x3xkk where 'x' is the V[x] register to compare against the value 'kk'
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op3(uint16_t opcode);

	/// <summary>
	/// Skips the next instruction if the register Vx is NOT equal to kk. Encoded as 0x4xkk where 'x' is the V[x] register to compare against the value 'kk'
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op4(uint16_t opcode);

	/// <summary>
	/// Skips the next instruction if the register Vx is equal to register Vy. Encodied as 0x5xy0 where 'X' is the V[x] register and Y is the V[y] register.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op5(uint16_t opcode);

	/// <summary>
	/// Set Vx register to the value specified by the OpCode. Encoded as 0x6xkk where 'x' is the V[x] register to put the value 'kk' into
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op6(uint16_t opcode);

	/// <summary>
	/// Add value to Vx register. Encoded as 0x7xkk where 'x' is the V[x] register to add the value 'kk' to.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op7(uint16_t opcode);

	/// <summary>
	/// Operates on the V[x] register depending on the final bit of the OpCode. Encoded as 0x8xyn where 'x' is the V[x] register and 'y' is the V[y] register. 
	/// 'n' sets which operation to perform:
	///		0 = Set V[x] to the same as V[y]
	///		1 = Perform a bitwise OR of V[x] and V[y]. Store the result in V[x]
	///		2 = Perform a bitwise AND of V[x] and V[y]. Store the result in V[x]
	///		3 = Perform a bitwise XOR of V[x] and V[y]. Store the result in V[x]
	///		4 = Add V[x] and V[y]. Store the result in V[x]. If the addition resulted in a Carry then V[15] is set to 1 (Otherwise it's set to 0).
	///		5 = Subtract V[x] and V[y]. Store the result in V[x]. If the value stored in V[x] is larger than the value stored in V[y] then V[15] is set to 1 (Otherwise it's set to 0]
	///		6 = If the least significant byte of V[y] is 1 set V[15] to 1 (Otherwise set it to 0). Divide the value stored in V[y] by 2 and store the result in V[x] (Subtraction is done as a right bit-shift)
	///		7 = Set V[15] to 1 if the value stored in V[y] is greater than the value stored in V[x]. Subtract V[x] from V[y] and store the result in V[x]
	///		E = If the most significant byte of V[y] is 1 set V[15] to 1 (Otherwise set it to 0). Multiply the value stored in V[y] by 2 and store the result in V[x] (Multiply is done as a left bit-shift)
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op8(uint16_t opcode);

	/// <summary>
	/// Skip the next instruction if register Vx is NOT equal to register Vy. Encoded as 0x9xy0 where 'x' is the V[x] register and 'y' is the V[y] register.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void Op9(uint16_t opcode);

	/// <summary>
	/// Sets the I register to be the value encoded in the OpCode. Encoded as 0xAnnn where 'nnn' is the value to store in I.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void OpA(uint16_t opcode);

	/// <summary>
	/// Jump to address + offset stored in V0 register. Encoded as 0xBnnn where 'nnn' is the base address to jump to.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void OpB(uint16_t opcode);

	/// <summary>
	/// Generate a random byte between 0-255 and AND it with specified value. Store the result in register Vx. Encoded as 0xCxkk where 'x' is the V[x] register to store the result in and 'kk' is the value to AND with the randomly generated byte.
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void OpC(uint16_t opcode);

	/// <summary>
	/// Draw n-byte sprite stored in memory location I to the screen and V[x] V[y]. Encoded as 0xDxyn where 'x' is the V[X] register which stores the X coordinate the sprite will be drawn to, 'y' is the V[y] register which stores the Y coordinate the sprite will
	/// be drawn to. Finally, 'n' is the number of bytes to read from memory (The memory address read is the value currently stored in I).
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void OpD(uint16_t opcode);

	/// <summary>
	/// Skips the next instruction depending on if the keyboard keycode stored in V[x] is pressed or not. encoded as 0xExnn where 'x' is the V[x] register and 'nn' is wether the key should be down or up. 
	/// Values for 'nn' can be:
	///		0x9E - Skips if the key is pressed
	///		0xA1 - Skips if the key is NOT pressed
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void OpE(uint16_t opcode);

	/// <summary>
	/// Multiple sub-instructions depending on the OpCode. Encoded as '0xFxnn' where 'x' is the V[x] register and 'nn' is the action to perform.
	/// Values for 'nn' and the operation performed are:
	///		0x07 - The value of the Delay Timer is stored into V[x]
	///		0x0A - Wait for a key press. Store the pressed keycode into V[x]
	///		0x15 - Set the Delay Timer to the value currently in V[x]
	///		0x18 - Set the Sound Timer to the value currently in V[x]
	///		0x1E - Add the value stored in V[x] to I
	///		0x29 - Set I to the location of the sprite referenced by V[x]
	///		0x33 - Store BCD representation of V[x] in memory locations I, I + 1 and I + 2
	///		0x55 - Store registers V[0] through to V[x] to memory, starting at the address stored in I
	///		0x65 - Read the registers V[0] through to V[x] from memory, starting at the address stored in I
	/// </summary>
	/// <param name="opcode">The OpCode to execute</param>
	void OpF(uint16_t opcode);
};