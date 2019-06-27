#ifndef APU_H
#define APU_H

#include <cstdint>

class APU {
public:
	APU();
	virtual ~APU();

public:
	virtual void setSW1C1(uint8_t val) {
		mSW1C1 = val;
	}
	virtual void setSW1C2(uint8_t val) {
		mSW1C2 = val;
	}
	virtual void setDMC1(uint8_t val) {
		mDMC1 = val;
	}
	virtual void setDMC2(uint8_t val) {
		mDMC2 = val;
	}
	virtual void setChCtrl(uint8_t val) {
		mChCtrl = val;
	}
	virtual void setFrameCounter(uint8_t val) {
		mFrameCounter = val;
	}

protected:
	uint8_t mSW1C1;        // 0x4000
	uint8_t mSW1C2;        // 0x4000
	uint8_t mDMC1;         // 0x4010
	uint8_t mDMC2;         // 0x4011
	uint8_t mChCtrl;
	uint8_t mFrameCounter;
};

#endif
