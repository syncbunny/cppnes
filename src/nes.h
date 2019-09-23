#ifndef NES_H
#define NES_H

#include "mapper.h"
#include "frame.h"
#include "cpu.h"
#include "ppu.h"
#include "pad.h"

class Renderer;
class Core;

class NES {
public:
	NES(Renderer* renderer = 0);
	virtual ~NES();

public:
	bool loadCartridge(const char* path);
	void loadCore(Core* c);
	void powerOn();
	void reset();
	void clock();
	PAD* getPAD() {
		return mPAD;
	}

protected:
	bool cartridgeHasTrainer();
	uint8_t getMapperNo();
	void dump6000();
	void coreDump(Core* c) const;

protected:
	Frame* mFrame;
	Mapper* mMapper;
	CPU* mCPU;
	PPU* mPPU;
	APU* mAPU;
	PAD* mPAD;
	uint8_t* mWRAM;
	uint8_t* mERAM;
	unsigned char* mCartridgeMem;

	int mDClockCPU;
	int mDClockPPU;
	int mDClockAPU;
	int mDMAWait;

	uint32_t mClocks;
};

#endif
