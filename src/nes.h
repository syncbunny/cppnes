#ifndef NES_H
#define NES_H

#include "mapper.h"
#include "cpu.h"
#include "ppu.h"
#include "pad.h"

class Renderer;

class NES {
public:
	NES(Renderer* renderer = 0);
	virtual ~NES();

public:
	bool loadCartridge(const char* path);
	void powerOn();
	void reset();
	void clock();

protected:
	bool cartridgeHasTrainer();
	uint8_t getMapperNo();

protected:
	Mapper* mMapper;
	CPU* mCPU;
	PPU* mPPU;
	APU* mAPU;
	PAD* mPAD;
	uint8_t* mWRAM;
	unsigned char* mCartridgeMem;

	int mDClockCPU;
	int mDClockPPU;
	int mDMAWait;
};

#endif
