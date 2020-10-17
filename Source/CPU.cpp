#include "CPU.h"

void CPU::Init()
{
	m_CpuState = new ChipState();

	m_CpuState->I = 0;

	m_CpuState->Delay = 0;
	m_CpuState->Sound = 0;

	m_CpuState->PC = 0x200;
	m_CpuState->SP = 0xFA0;

	m_CpuState->Memory = (uint8_t*)calloc(4096, 1);

	if (m_CpuState->Memory != nullptr)
	{
		m_CpuState->VideoMemory = (uint8_t*)calloc(2048, 1); //&m_CpuState->Memory[0xF00];

		memcpy(&m_CpuState->Memory[Sprites::FONT_START], Sprites::Font, Sprites::FONT_SIZE);
	}
}

void CPU::Stop()
{
	std::cout << "INFO: CPU Stop called!" << std::endl;

	m_CpuState->bIsStopped = true;
}

bool CPU::LoadProgram(const wchar_t* FilePath)
{
	std::ifstream inputFile;
	inputFile.open(FilePath, std::ios::binary);

	if (inputFile.is_open())
	{
		inputFile.seekg(0, std::ios::end);
		std::streampos end = inputFile.tellg();

		inputFile.seekg(0, std::ios::beg);
		std::streampos begin = inputFile.tellg();

		int fileSize = static_cast<int>(end - begin);

		char* inBuffer = new char[fileSize];

		inputFile.read(inBuffer, fileSize);
		inputFile.close();

		uint8_t* memBuffer = &m_CpuState->Memory[0x200];

		memcpy(memBuffer, inBuffer, fileSize);

		delete[] inBuffer;

		m_CpuState->bIsStopped = false;

		return true;
	}

	std::cout << "ERROR: Failed to open input '" << *FilePath << "': " << strerror(errno) << std::endl;

	return false;
}

void CPU::RunCycle()
{
	if (m_CpuState->bIsStopped) return;

	uint8_t* opcode = &m_CpuState->Memory[m_CpuState->PC];

	uint8_t highBit = opcode[0] >> 4;

	switch (highBit)
	{
		case 0x00: Op0(opcode); break;
		case 0x01: Op1(opcode); break;
		case 0x02: Op2(opcode); break;
		case 0x03: Op3(opcode); break;
		case 0x04: Op4(opcode); break;
		case 0x05: Op5(opcode); break;
		case 0x06: Op6(opcode); break;
		case 0x07: Op7(opcode); break;
		case 0x08: Op8(opcode); break;
		case 0x09: Op9(opcode); break;
		case 0x0A: OpA(opcode); break;
		case 0x0B: OpB(opcode); break;
		case 0x0C: OpC(opcode); break;
		case 0x0D: OpD(opcode); break;
		case 0x0E: OpE(opcode); break;
		case 0x0F: OpF(opcode); break;
		default:
		{
			std::cout << "ERROR: Unknown OpCode: 0x" << std::hex << std::setw(2) << static_cast<int>(highBit) << std::endl;
			break;
		}
	}
}

void CPU::SetKeyState(uint8_t keycode)
{
	m_CpuState->KeyState[keycode] = 0x1;
}

void CPU::ClearKeyState(uint8_t keycode)
{
	m_CpuState->KeyState[keycode] = 0x0;
}

void CPU::SetDelayRegister(uint8_t value)
{
	m_CpuState->Delay = value;
}

void CPU::SetSoundRegister(uint8_t value)
{
	m_CpuState->Sound = value;
}

const ChipState* CPU::GetState() const
{
	return m_CpuState;
}

void CPU::Op0(uint8_t* opcode)
{
	if (opcode[1] == 0xE0)
	{
		memset(m_CpuState->VideoMemory, 0, 256);

		m_CpuState->PC += 2;
	}
	else if (opcode[1] == 0xEE)
	{
		/* TODO - VALIDATE */
		uint16_t address = (m_CpuState->Memory[m_CpuState->SP] << 8) | m_CpuState->Memory[m_CpuState->SP + 1];
		
		m_CpuState->SP += 2;

		m_CpuState->PC = address;
	}
	else
	{
		std::cout << "ERROR: Unknown operation for Op0: 0x" << std::hex << std::setw(2) << static_cast<int>(opcode[1]) << std::endl;
		Stop();

		return;
	}
}

void CPU::Op1(uint8_t* opcode)
{
	uint16_t address = ((opcode[0] & 0xF) << 8) | opcode[1];

	m_CpuState->PC = address;
}

void CPU::Op2(uint8_t* opcode)
{
	uint16_t address = ((opcode[0] & 0xF) << 8) | opcode[1];

	m_CpuState->SP -= 2;

	m_CpuState->Memory[m_CpuState->SP] = ((m_CpuState->PC + 2) & 0xFF00) >> 8;
	m_CpuState->Memory[m_CpuState->SP + 1] = (m_CpuState->PC + 2) & 0xFF;

	m_CpuState->PC = address;
}

void CPU::Op3(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	if (m_CpuState->V[registerIdx] == opcode[1])
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::Op4(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	if (m_CpuState->V[registerIdx] != opcode[1])
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::Op5(uint8_t* opcode)
{
	uint8_t xIdx = opcode[0] & 0xF;
	uint8_t yIdx = (opcode[1] & 0xF0) >> 4;

	if (m_CpuState->V[xIdx] == m_CpuState->V[yIdx])
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::Op6(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	m_CpuState->V[registerIdx] = opcode[1];

	m_CpuState->PC += 2;
}

void CPU::Op7(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	m_CpuState->V[registerIdx] += opcode[1];

	m_CpuState->PC += 2;
}

void CPU::Op8(uint8_t* opcode)
{
	uint8_t lowBit = opcode[1] & 0xF;

	uint8_t xIdx = opcode[0] & 0xF;
	uint8_t yIdx = (opcode[1] & 0xF0) >> 4;

	switch (lowBit)
	{
		case 0x0:
		{
			m_CpuState->V[xIdx] = m_CpuState->V[yIdx];
			break;
		}
		case 0x1:
		{
			m_CpuState->V[xIdx] |= m_CpuState->V[yIdx];
			break;
		}
		case 0x2:
		{
			m_CpuState->V[xIdx] &= m_CpuState->V[yIdx];
			break;
		}
		case 0x3:
		{
			m_CpuState->V[xIdx] ^= m_CpuState->V[yIdx];
			break;
		}
		case 0x4:
		{
			uint16_t result = m_CpuState->V[xIdx] + m_CpuState->V[yIdx];

			if (result & 0xFF00)
			{
				m_CpuState->V[0xF] = 1;
			}
			else
			{
				m_CpuState->V[0xF] = 0;
			}

			m_CpuState->V[xIdx] = result & 0xFF;

			break;
		}
		case 0x5:
		{
			int bIsBorrow = (m_CpuState->V[xIdx] > m_CpuState->V[yIdx]);
			
			m_CpuState->V[xIdx] -= m_CpuState->V[yIdx];

			m_CpuState->V[0xF] = bIsBorrow;

			break;
		}
		case 0x6:
		{
			/* TODO: 
			 * Documentation on this instruction super patchy. Some places say the instruction only operates on Vx, others say this is incorrect and it actually operates on Vy, storing the result in Vx.
			 * Need to review this once up and running to determine which is correct. Currently implemented as: 
					VF = Vy & 0x1; 
					Vx = Vy >> 1;

			 * Alternative is:
					VF = Vx & 0x1;
					Vx = Vx >> 1;
			 */
			
			m_CpuState->V[0xF] = m_CpuState->V[xIdx] & 0x1;

			m_CpuState->V[xIdx] = m_CpuState->V[xIdx] >> 1;

			break;
		}
		case 0x7:
		{
			int bIsBorrow = (m_CpuState->V[yIdx] > m_CpuState->V[xIdx]);

			m_CpuState->V[xIdx] = (m_CpuState->V[yIdx] - m_CpuState->V[xIdx]);

			m_CpuState->V[0xF] = bIsBorrow;

			break;
		}
		case 0xE:
		{
			/* TODO:
			 * Documentation on this instruction super patchy. Some places say the instruction only operates on Vx, others say this is incorrect and it actually operates on Vy, storing the result in Vx.
			 * Need to review this once up and running to determine which is correct. Currently implemented as:
					VF = Vy & 0x80;
					Vx = Vy << 1;

			 * Alternative is:
					VF = Vx & 0x80;
					Vx = Vx << 1;
			 */

			m_CpuState->V[0xF] = ((m_CpuState->V[xIdx] & 0x80) == 0x80);

			m_CpuState->V[xIdx] = m_CpuState->V[xIdx] << 1;

			break;
		}
		default:
		{
			std::cout << "ERROR: Unknown operation for Op8: 0x" << std::hex << std::setw(2) << static_cast<int>(lowBit) << std::endl;
			Stop();

			return;
		}
	}
	m_CpuState->PC += 2;
}

void CPU::Op9(uint8_t* opcode)
{
	uint8_t xIdx = opcode[0] & 0xF;
	uint8_t yIdx = (opcode[1] & 0xF0) >> 4;

	if (m_CpuState->V[xIdx] != m_CpuState->V[yIdx])
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::OpA(uint8_t* opcode)
{
	m_CpuState->I = ((opcode[0] & 0xF) << 8) | opcode[1];

	m_CpuState->PC += 2;
}

void CPU::OpB(uint8_t* opcode)
{
	uint16_t address = ((uint16_t)m_CpuState->V[0]) + (((opcode[0] & 0xF) << 8) | opcode[1]);

	m_CpuState->PC = address;
}

void CPU::OpC(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	m_CpuState->V[registerIdx] = (rand() % 256) & opcode[1];

	m_CpuState->PC += 2;
}

void CPU::OpD(uint8_t* opcode)
{
	uint8_t xRegister = opcode[0] & 0xF;
	uint8_t yRegister = opcode[1] & 0x0F;

	uint16_t spriteX = m_CpuState->V[xRegister >> 8];
	uint16_t spriteY = m_CpuState->V[yRegister >> 4];

	uint16_t height = opcode[1] & 0xF;

	for (int row = 0; row < height; row++)
	{
		uint16_t pixel = m_CpuState->Memory[m_CpuState->I + row];
		
		for (int col = 0; col < 8; col++)
		{
			if ((pixel & (0x80 >> col)) != 0)
			{
				m_CpuState->V[0xF] = 1;
			}
			m_CpuState->VideoMemory[spriteX + col + ((spriteY + row) * 64)] ^= 1;
		}
	}

	m_CpuState->PC += 2;

	/*
	uint16_t spriteSize = opcode[1] & 0xF;

	uint8_t xIdx = opcode[0] & 0xF;
	uint8_t yIdx = (opcode[1] & 0xF0) >> 4;

	uint16_t xPosition = m_CpuState->V[xIdx];
	uint16_t yPosition = m_CpuState->V[yIdx];

	m_CpuState->V[0xF] = 0;

	for (uint16_t row = 0; row < spriteSize; row++)
	{
		uint8_t* sprite = &m_CpuState->Memory[m_CpuState->I + row];

		int spriteBits = 7;

		for (uint16_t col = xPosition; col < xPosition + 8 && col < 64; col++)
		{
			int jo = col / 8;
			int jm = col % 8;

			uint8_t srcBit = (*sprite >> spriteBits) & 0x1;

			if (srcBit)
			{
				uint8_t* destByte = &m_CpuState->VideoMemory[(row + yPosition) * (64 / 8) + jo];
				uint8_t destMask = (0x80) >> jm;
				uint8_t destBit = *destByte & destMask;

				srcBit = srcBit << (7 - jm);

				if (srcBit & destBit)
				{
					m_CpuState->V[0xF] = 1;
				}

				destBit ^= srcBit;

				*destByte = (*destByte & ~destMask) | destBit;
			}
			spriteBits--;
		}
	}
	
	m_CpuState->PC += 2;
	*/
}

void CPU::OpE(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	uint8_t keyState = m_CpuState->KeyState[m_CpuState->V[registerIdx]];

	if (opcode[1] == 0x9E)
	{
		if (keyState != 0x00)
		{
			m_CpuState->PC += 2;
		}
	}
	else if (opcode[1] == 0xA1)
	{
		if (keyState == 0x00)
		{
			m_CpuState->PC += 2;
		}
	}
	else
	{
		std::cout << "ERROR: Unknown operation for OpE: 0x" << std::hex << std::setw(2) << static_cast<int>(opcode[1]) << std::endl;
		Stop();

		return;
	}

	m_CpuState->PC += 2;
}

void CPU::OpF(uint8_t* opcode)
{
	uint8_t registerIdx = opcode[0] & 0xF;

	switch (opcode[1])
	{
		case 0x07:
		{
			m_CpuState->V[registerIdx] = m_CpuState->Delay;
			break;
		}
		case 0x0A:
		{
			if (!m_CpuState->bIsWaitingForKeyPress)
			{
				m_CpuState->bIsWaitingForKeyPress = true;

				memcpy(m_CpuState->PreviousKeyState, m_CpuState->KeyState, 16);

				return;
			}
			else
			{
				for (uint8_t i = 0; i < 16; i++)
				{
					if (m_CpuState->PreviousKeyState[i] == 0 && m_CpuState->KeyState[i] == 1)
					{
						m_CpuState->bIsWaitingForKeyPress = false;

						m_CpuState->V[registerIdx] = i;

						m_CpuState->PC += 2;
						return;
					}

					m_CpuState->PreviousKeyState[i] = m_CpuState->KeyState[i]; // In case anything got released
				}

				return;
			}
			break;
		}
		case 0x15:
		{
			m_CpuState->Delay = m_CpuState->V[registerIdx];
			break;
		}
		case 0x18:
		{
			m_CpuState->Sound = m_CpuState->V[registerIdx];
			break;
		}
		case 0x1E:
		{
			m_CpuState->I += m_CpuState->V[registerIdx];
			break;
		}
		case 0x29:
		{
			m_CpuState->I = Sprites::FONT_START + (m_CpuState->V[registerIdx] * 5);
			break;
		}
		case 0x33:
		{
			uint8_t value = m_CpuState->V[registerIdx];

			uint8_t ones;
			uint8_t tens;
			uint8_t hundreds;

			ones = value % 10;
			
			value /= 10;
			tens = value % 10;

			hundreds = value / 10;

			m_CpuState->Memory[m_CpuState->I] = hundreds;
			m_CpuState->Memory[m_CpuState->I + 1] = tens;
			m_CpuState->Memory[m_CpuState->I + 2] = ones;

			break;
		}
		case 0x55:
		{
			uint16_t offset = m_CpuState->I;

			for (uint8_t i = 0; i <= registerIdx; i++)
			{
				m_CpuState->Memory[offset + i] = m_CpuState->V[i];
			}

			m_CpuState->I += (registerIdx + 1);

			break;
		}
		case 0x65:
		{
			uint16_t offset = m_CpuState->I;

			for (uint8_t i = 0; i <= registerIdx; i++)
			{
				m_CpuState->V[i] = m_CpuState->Memory[offset + i];
			}

			m_CpuState->I += (registerIdx + 1);
			break;
		}
		default:
		{
			std::cout << "ERROR: Unknown operation for OpF: 0x" << std::hex << std::setw(2) << static_cast<int>(opcode[1]) << std::endl;
			Stop();

			return;
		}
	}

	m_CpuState->PC += 2;
}