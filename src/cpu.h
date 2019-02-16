#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include "mapper.h"

class CPU {
public:
	CPU(Mapper* mapper);
	virtual ~CPU();

public:
	void powerOn();
	void reset();
	void clock();
	void dump();

protected:
	void updateP(uint8_t n);
	void doReset();

protected:
	Mapper* mMapper;
	uint8_t	mA;
	uint8_t mX, mY;
	uint8_t mS;
	uint8_t mP;
	uint16_t mPC;

	int mClockRemain;

	bool mResetFlag;
};

#endif
