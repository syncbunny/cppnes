#ifndef PPU_H
#define PPU_H

#include <cstdint>

class PPU {
public:
	PPU();
	virtual ~PPU();

public:
	virtual void setCR1(uint8_t v) {
		mCR1 = v;
	}
	virtual void setCR2(uint8_t v) {
		mCR2 = v;
	}
	virtual uint8_t getCR1() const {
		return mCR1;
	}
	virtual uint8_t getCR2() const {
		return mCR2;
	}
	virtual uint8_t* getSpriteMemAddr() {
		return &mSpriteMem[0];
	}
	virtual uint8_t getSR();
	virtual uint8_t* getMemory() {
		return mMem;
	}
	virtual void setScroll(uint8_t val);
	virtual void setWriteAddr(uint8_t a);
	virtual void write(uint8_t val);
	virtual void clock();

protected:
	virtual void startVR();
	virtual void frameStart();
	virtual void frameEnd();
	virtual void renderSprite(int y);

protected:
	uint8_t mCR1; // Control Register
	uint8_t mCR2; // Control Register
	uint8_t mSR;  // Status Register
	uint8_t mScrollOffsetTarget;
	uint16_t mWriteAddr;
	uint16_t mScrollVH; // Hi=V, Low=H
	uint16_t mLine;
	uint16_t mLineClock;
	uint32_t mFrames;

	uint8_t* mMem;
	uint8_t mSpriteMem[256];
	uint8_t* mScreen;
};

#endif
