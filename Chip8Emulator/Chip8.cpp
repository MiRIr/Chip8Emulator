#include <cstring>
#include <fstream>
#include <time.h>
#include <Windows.h>
#include "Chip8.h"

Chip8::Chip8(FILE* f)
{
	srand(time(NULL));
	OP = I = SP = delayT = soundT = 0;
	PC = 0x200;

	memset(G, 0, sizeof(G));
	memset(S, 0, sizeof(S));
	memset(M, 0, sizeof(M));
	memset(V, 0, sizeof(V));
	memset(K, 0, sizeof(K));

	{
		uchar font[80] =
		{
			0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
			0x20, 0x60, 0x20, 0x20, 0x70, // 1
			0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
			0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
			0x90, 0x90, 0xF0, 0x10, 0x10, // 4
			0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
			0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
			0xF0, 0x10, 0x20, 0x40, 0x40, // 7
			0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
			0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
			0xF0, 0x90, 0xF0, 0x90, 0x90, // A
			0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
			0xF0, 0x80, 0x80, 0x80, 0xF0, // C
			0xE0, 0x90, 0x90, 0x90, 0xE0, // D
			0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
			0xF0, 0x80, 0xF0, 0x80, 0x80  // F
		};
		memcpy(M, font, 80);
	}

	fread(M + PC, 1, 0xDFF, f);
	fclose(f);
}

void Chip8::cycle(int steps)
{
	for (int step = 0; step < 10; ++step)
	{
		OP = M[PC] << 8 | M[PC + 1];
		PC += 2;

		int x = getNibble(2), y = getNibble(1);
		switch (getNibble(3))
		{
		case 0x0:
			switch (getSlab())
			{
			case 0x0E0:
				memset(G, 0, sizeof(G));
				draw = true;
				break;
			case 0x0EE:
				PC = S[--SP];
				break;
			}
			break;
		case 0x1:
			PC = getSlab();
			break;
		case 0x2:
			S[SP++] = PC;
			PC = getSlab();
			break;
		case 0x3:
			if (V[x] == getByte(0))
				PC += 2;
			break;
		case 0x4:
			if (V[x] != getByte(0))
				PC += 2;
			break;
		case 0x5:
			if (V[x] == V[y] && getNibble(0) == 0)
				PC += 2;
			break;
		case 0x6:
			V[x] = getByte(0);
			break;
		case 0x7:
			V[x] += getByte(0);
			break;
		case 0x8:
			switch (getNibble(0))
			{
			case 0x0:
				V[x] = V[y];
				break;
			case 0x1:
				V[x] |= V[y];
				break;
			case 0x2:
				V[x] &= V[y];
				break;
			case 0x3:
				V[x] ^= V[y];
				break;
			case 0x4:
				V[15] = (V[x] > 0xFF - V[y]);
				V[x] += V[y];
				break;
			case 0x5:
				V[15] = V[x] > V[y];
				V[x] -= V[y];
				break;
			case 0x6:
				V[15] = V[x] & 1;
				V[x] >>= 1;
				break;
			case 0x7:
				V[15] = V[y] > V[x];
				V[x] = V[y] - V[x];
				break;
			case 0xE:
				V[15] = V[x] >> 7;
				V[x] <<= 1;
				break;
			}
			break;
		case 0x9:
			if (V[x] != V[y] && getNibble(0) == 0)
				PC += 2;
			break;
		case 0xA:
			I = getSlab();
			break;
		case 0xB:
			PC = V[0] + getSlab();
			break;
		case 0xC:
			V[x] = (rand() % 256) & getByte(0);
			break;
		case 0xD:
			V[15] = 0;
			draw = 1;
			for (ushort yG = 0; yG < getNibble(0); ++yG)
			{
				uchar mY = M[I + yG];
				for (ushort xG = 0; xG < 8; ++xG)
				{
					if ((mY & (0x80 >> xG)) != 0)
					{
						ushort pixel = ((V[x] + xG) + ((V[y] + yG) << 6)) % 2048;
						V[15] |= G[pixel] & 1;
						G[pixel] = ~G[pixel];
					}
				}
			}
			break;
		case 0xE:
			switch (getByte(0))
			{
			case 0x9E:
				if (K[V[x]])
					PC += 2;
				break;
			case 0xA1:
				if (!K[V[x]])
					PC += 2;
				break;
			}
			break;
		case 0xF:
			switch (getByte(0))
			{
			case 0x07:
				V[x] = delayT;
				break;
			case 0x0A:
				PC -= 2;
				for (int i = 0; i < sizeof(K); ++i)
					if (K[i])
					{
						V[x] = i;
						PC += 2;
						break;
					}
				break;
			case 0x15:
				delayT = V[x];
				break;
			case 0x18:
				soundT = V[x];
				break;
			case 0x1E:
				V[15] = (I > 0xFFF - V[x]);
				I += V[x];
				break;
			case 0x29:
				I = V[x] * 5;
				break;
			case 0x33:
				M[I] = V[x] / 100;
				M[I + 1] = (V[x] / 10) % 10;
				M[I + 2] = V[x] % 10;
				break;
			case 0x55:
				for (int i = 0; i <= x; ++i)
					M[I++] = V[i];
				break;
			case 0x65:
				for (int i = 0; i <= x; ++i)
					V[i] = M[I++];
				break;
			}
			break;
		}
	}

	if (delayT)
		--delayT;
	if (soundT)
		--soundT;
}

char Chip8::canDraw()
{
	char d = draw;
	draw = 0;
	return d;
}

int* Chip8::getDisplay()
{
	return G;
}

void Chip8::setKey(int k, bool pressed)
{
	K[k] = pressed;
}

char Chip8::playSound()
{
	return soundT;
}

int Chip8::getNibble(int nibble)
{
	return (OP >> (nibble << 2)) & 0xF;
}

int Chip8::getByte(int byte)
{
	return (OP >> (byte << 3)) & 0xFF;
}

int Chip8::getSlab()
{
	return OP & 0x0FFF;
}