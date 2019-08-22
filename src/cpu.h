#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include "mapper.h"

struct APvc {
	uint8_t a;
	uint8_t p_vc;
};

class Core;

class CPU {
public:
	CPU(Mapper* mapper);
	virtual ~CPU();

public:
	void powerOn();
	void reset();
	void nmi();
	void clock();
	void startDMA();
	void coreDump(Core* c) const;
	void loadCore(Core* c);
	void dump();

protected:
	void updateP(uint8_t n);
	void doReset();
	void doNMI();
	void buildADC_APvcTable();
	void buildSBC_APvcTable();

protected:
	Mapper* mMapper;
	uint8_t	mA;
	uint8_t mX, mY;
	uint8_t mS;
	uint8_t mP;
	uint16_t mPC;

	int mClockRemain;

	bool mResetFlag;
	bool mNMIFlag;
	bool mBRKFlag;

	APvc *mADC_APvcTable;
	APvc *mSBC_APvcTable;
};

#endif
