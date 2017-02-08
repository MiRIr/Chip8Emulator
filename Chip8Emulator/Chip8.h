#pragma once

class Chip8
{
public:
	Chip8(FILE* f);
	void cycle(int steps);
	char canDraw();
	int* getDisplay();
	void setKey(int x, bool pressed);
	char playSound();
private:
	typedef unsigned short ushort;
	typedef unsigned char uchar;

	int G[2048];
	ushort S[16], OP, I, PC;
	uchar M[4096], V[16], K[16], SP, delayT, soundT, draw;

	int getNibble(int nibble);
	int getByte(int byte);
	int getSlab();
};