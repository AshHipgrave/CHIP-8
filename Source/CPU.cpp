#include "CPU.h"

void CPU::Init()
{
	m_CpuState = new ChipState();

	m_CpuState->I = 0;

	m_CpuState->Delay = 0;
	m_CpuState->Sound = 0;

	m_CpuState->PC = 0x200;
	m_CpuState->SP = 0;

	for (int i = 0; i < 4096; i++)
	{
		m_CpuState->Memory[i] = 0;
	}

	for (int i = 0; i < 2048; i++)
	{
		m_CpuState->VideoMemory[i] = 0;
	}

	for (int i = 0; i < 16; i++)
	{
		m_CpuState->V[i] = 0;
		m_CpuState->Stack[i] = 0;
		m_CpuState->KeyState[i] = 0;
		m_CpuState->PreviousKeyState[i] = 0;
	}

	srand(time(NULL));

	//m_CpuState->VideoMemory = (uint8_t*)calloc(2048, 1); //&m_CpuState->Memory[0xF00];

	//memcpy(&m_CpuState->Memory[Sprites::FONT_START], Sprites::Font, Sprites::FONT_SIZE);
}

void CPU::SetKeyState(uint8_t keycode)
{
	m_CpuState->KeyState[keycode] = 1;
}

void CPU::ClearKeyState(uint8_t keycode)
{
	m_CpuState->KeyState[keycode] = 0;
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

		for (int i = 0; i < fileSize; i++)
		{
			m_CpuState->Memory[i + 512] = (uint8_t)inBuffer[i];
		}

		//uint8_t* memBuffer = &m_CpuState->Memory[0x200];

		//memcpy(memBuffer, inBuffer, fileSize);

		//delete[] inBuffer;

		m_CpuState->bIsStopped = false;

		return true;
	}

	std::cout << "ERROR: Failed to open input '" << *FilePath << "': " << strerror(errno) << std::endl;

	return false;
}

void CPU::RunCycle()
{
	if (m_CpuState->bIsStopped) return;

	uint16_t opcode = m_CpuState->Memory[m_CpuState->PC] << 8 | m_CpuState->Memory[m_CpuState->PC + 1];

	uint16_t highBit = opcode & 0xF000;

	switch (highBit)
	{
		case 0x0000: Op0(opcode); break;
		case 0x1000: Op1(opcode); break;
		case 0x2000: Op2(opcode); break;
		case 0x3000: Op3(opcode); break;
		case 0x4000: Op4(opcode); break;
		case 0x5000: Op5(opcode); break;
		case 0x6000: Op6(opcode); break;
		case 0x7000: Op7(opcode); break;
		case 0x8000: Op8(opcode); break;
		case 0x9000: Op9(opcode); break;
		case 0xA000: OpA(opcode); break;
		case 0xB000: OpB(opcode); break;
		case 0xC000: OpC(opcode); break;
		case 0xD000: OpD(opcode); break;
		case 0xE000: OpE(opcode); break;
		case 0xF000: OpF(opcode); break;
		default:
		{
			std::cout << "ERROR: Unknown OpCode: 0x" << std::hex << std::setw(2) << static_cast<int>(highBit) << std::endl;
			Stop();

			break;
		}
	}
}

void CPU::Op0(uint16_t opcode)
{
	switch (opcode & 0x000F)
	{
		case 0x0000:
		{
			memset(m_CpuState->VideoMemory, 0, 2048);

			m_CpuState->PC += 2;
			break;
		}
		case 0x000E:
		{
			m_CpuState->SP--;

			m_CpuState->PC = m_CpuState->Stack[m_CpuState->SP];

			m_CpuState->PC += 2; //TODO: Check this
			break;
		}
		default:
		{
			std::cout << "ERROR: Unknown operation for Op0: 0x" << std::hex << static_cast<int>(opcode) << std::endl;
			Stop();

			return;
		}
	}
}

void CPU::Op1(uint16_t opcode)
{
	m_CpuState->PC = opcode & 0x0FFF;
}

void CPU::Op2(uint16_t opcode)
{
	m_CpuState->Stack[m_CpuState->SP] = m_CpuState->PC;
	m_CpuState->SP++;

	m_CpuState->PC = opcode & 0xFFFF;
}

void CPU::Op3(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;

	uint8_t value = opcode & 0x00FF;

	if (m_CpuState->V[registerIdx] == value)
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::Op4(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;

	uint8_t value = opcode & 0x00FF;

	if (m_CpuState->V[registerIdx] != value)
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::Op5(uint16_t opcode)
{
	uint8_t xIdx = (opcode & 0x0F00) >> 8;
	uint8_t yIdx = (opcode & 0x00FF) >> 4;

	if (m_CpuState->V[xIdx] == m_CpuState->V[yIdx])
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::Op6(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;
	uint8_t value = opcode & 0x00FFF;

	m_CpuState->V[registerIdx] = value;

	m_CpuState->PC += 2;
}

void CPU::Op7(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;
	uint8_t value = opcode & 0x00FF;

	m_CpuState->V[registerIdx] += value;

	m_CpuState->PC += 2;
}

void CPU::Op8(uint16_t opcode)
{
	uint8_t lowBit = opcode & 0x000F;

	uint8_t xIdx = (opcode & 0x0F00) >> 8;
	uint8_t yIdx = (opcode & 0x00F0) >> 4;

	switch (lowBit)
	{
		case 0x0000:
		{
			m_CpuState->V[xIdx] = m_CpuState->V[yIdx];

			break;
		}
		case 0x0001:
		{
			m_CpuState->V[xIdx] |= m_CpuState->V[yIdx];
			
			break;
		}
		case 0x0002:
		{
			m_CpuState->V[xIdx] &= m_CpuState->V[yIdx];

			break;
		}
		case 0x0003:
		{
			m_CpuState->V[xIdx] ^= m_CpuState->V[yIdx];

			break;
		}
		case 0x0004:
		{
			m_CpuState->V[xIdx] += m_CpuState->V[yIdx];

			if (m_CpuState->V[xIdx] > (0xFF - m_CpuState->V[yIdx]))
			{
				m_CpuState->V[0xF] = 1;
			}
			else
			{
				m_CpuState->V[0xF] = 0;
			}

			break;
		}
		case 0x0005:
		{
			bool bIsBorrow = (m_CpuState->V[xIdx] > m_CpuState->V[yIdx]);
			
			m_CpuState->V[xIdx] -= m_CpuState->V[yIdx];

			m_CpuState->V[0xF] = bIsBorrow ? 0 : 1;

			break;
		}
		case 0x0006:
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
			
			m_CpuState->V[0xF] = m_CpuState->V[yIdx] & 0x1;

			m_CpuState->V[xIdx] = m_CpuState->V[yIdx] >> 1;

			break;
		}
		case 0x0007:
		{
			bool bIsBorrow = (m_CpuState->V[yIdx] > m_CpuState->V[xIdx]);

			m_CpuState->V[xIdx] = (m_CpuState->V[yIdx] - m_CpuState->V[xIdx]);

			m_CpuState->V[0xF] = bIsBorrow ? 0 : 1;

			break;
		}
		case 0x000E:
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

			m_CpuState->V[0xF] = m_CpuState->V[yIdx] >> 7;

			m_CpuState->V[xIdx] = m_CpuState->V[yIdx] << 1;

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

void CPU::Op9(uint16_t opcode)
{
	uint8_t xIdx = (opcode & 0x0F00) >> 8;
	uint8_t yIdx = (opcode & 0x00F0) >> 4;

	if (m_CpuState->V[xIdx] != m_CpuState->V[yIdx])
	{
		m_CpuState->PC += 2;
	}
	m_CpuState->PC += 2;
}

void CPU::OpA(uint16_t opcode)
{
	m_CpuState->I = opcode & 0x0FFF;

	m_CpuState->PC += 2;
}

void CPU::OpB(uint16_t opcode)
{
	uint16_t address = (opcode & 0x0FFF) + m_CpuState->V[0];

	m_CpuState->PC = address;
}

void CPU::OpC(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;

	m_CpuState->V[registerIdx] = (rand() % 256) & (opcode & 0x00FF);

	m_CpuState->PC += 2;
}

void CPU::OpD(uint16_t opcode)
{
	uint8_t xRegister = (opcode & 0x0F00) >> 8;
	uint8_t yRegister = (opcode & 0x00F0) >> 4;

	uint16_t spriteX = m_CpuState->V[xRegister];
	uint16_t spriteY = m_CpuState->V[yRegister];

	uint16_t height = opcode & 0x000F;

	m_CpuState->V[0xF] = 0;

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
}

void CPU::OpE(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;

	uint8_t keyState = m_CpuState->KeyState[m_CpuState->V[registerIdx]];

	uint16_t lowByte = opcode & 0x00FF;

	switch (lowByte)
	{
		case 0x009E:
		{
			if (keyState != 0)
			{
				m_CpuState->PC += 2;
			}
			break;
		}
		case 0x00A1:
		{
			if (keyState == 0)
			{
				m_CpuState->PC += 2;
			}
			break;
		}
		default:
		{
			std::cout << "ERROR: Unknown operation for OpE: 0x" << std::hex << std::setw(2) << static_cast<int>(opcode) << std::endl;
			Stop();

			return;
		}
	}

	m_CpuState->PC += 2;
}

void CPU::OpF(uint16_t opcode)
{
	uint8_t registerIdx = (opcode & 0x0F00) >> 8;

	uint16_t lowByte = opcode & 0x00FF;

	switch (lowByte)
	{
		case 0x0007:
		{
			m_CpuState->V[registerIdx] = m_CpuState->Delay;
			break;
		}
		case 0x000A:
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
		case 0x0015:
		{
			m_CpuState->Delay = m_CpuState->V[registerIdx];
			break;
		}
		case 0x0018:
		{
			m_CpuState->Sound = m_CpuState->V[registerIdx];
			break;
		}
		case 0x001E:
		{
			if (m_CpuState->I + m_CpuState->V[registerIdx] > 0xFFF)
			{
				m_CpuState->V[0xF] = 1;
			}
			else
			{
				m_CpuState->V[0xF] = 0;
			}

			m_CpuState->I += m_CpuState->V[registerIdx];
			break;
		}
		case 0x0029:
		{
			m_CpuState->I = Sprites::FONT_START + (m_CpuState->V[registerIdx] * 0x5);
			break;
		}
		case 0x0033:
		{
			uint8_t value = m_CpuState->V[registerIdx];

			m_CpuState->Memory[m_CpuState->I] = value / 100;
			m_CpuState->Memory[m_CpuState->I + 1] = (value / 10) % 10;
			m_CpuState->Memory[m_CpuState->I + 2] = value % 10;

			break;
		}
		case 0x0055:
		{
			uint16_t offset = m_CpuState->I;

			for (uint8_t i = 0; i <= registerIdx; i++)
			{
				m_CpuState->Memory[offset + i] = m_CpuState->V[i];
			}

			m_CpuState->I += (registerIdx + 1);

			break;
		}
		case 0x0065:
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
			std::cout << "ERROR: Unknown operation for OpF: 0x" << std::hex << std::setw(2) << static_cast<int>(opcode) << std::endl;
			Stop();

			return;
		}
	}

	m_CpuState->PC += 2;
}