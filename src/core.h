#ifndef CORE_H
#define CORE_H

#include <cstdint>
class CPU;

struct CPUCore {
	uint8_t a, x, y, s, p;
	uint16_t pc;

	int clockRemain;
	bool resetFlag;
	bool NMIFlag;
	bool IRQFlag;
	bool BRKFlag;
};

struct PPUCore {
	uint8_t cr1;
	uint8_t cr2;
	uint8_t sr;
	uint8_t scrollOffsetTarget;
	uint16_t writeAddr;
	uint8_t spriteMemAddr;
	uint8_t mirror;
	uint16_t line;
	uint16_t lineClock;
	uint32_t frames;
	uint8_t writeMode;
	uint8_t t1;
	uint8_t t2;
	uint8_t v1;
	uint8_t v2;
	uint8_t fineX;

	uint8_t mem[0x4000];
	uint8_t spriteMem[256];
	uint8_t screen[256*240*3];
	uint8_t stencil[256*240];

	uint8_t readBuffer;
	uint16_t lastBGNameTableAddr;
	uint8_t lastPaletteId;
};

class Core {
public:
	Core();
	~Core();

public:
	void setCPU(struct CPUCore& c);
	void setPPU(struct PPUCore& c);
	void setWRAM(uint8_t* WRAM);
	const struct CPUCore getCPU() const;
	const struct PPUCore getPPU() const;
	const uint8_t* getWRAM() const;
	void dump(const char* path);
	static Core* load(const char* path);

protected:
	struct CPUCore mCPUCore;
	struct PPUCore mPPUCore;
	uint8_t mWRAM[0x0800];
};

#endif
