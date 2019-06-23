#ifndef PPU_H
#define PPU_H

#include <cstdint>

struct Palette {
        uint8_t col[4]; // col[0]: clear
} __attribute__((packed)) ;

class PPU {
public:
	enum {
		MIRROR_H, MIRROR_V
	};
public:
	PPU();
	virtual ~PPU();

public:
	virtual void setMirror(int m) {
		mMirror = m;
	}
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
	virtual	void renderBG(int x, int y);
	virtual void renderSprite(int y);
	virtual void getColor(uint8_t* base, uint8_t pat, const struct Palette* paletteP, uint8_t u, uint8_t v, uint8_t* rgb);
	virtual struct Palette* getPalette(uint8_t* base, uint8_t x, uint8_t y);

protected:
	uint8_t mCR1; // Control Register
	uint8_t mCR2; // Control Register
	uint8_t mSR;  // Status Register
	uint8_t mScrollOffsetTarget;
	uint16_t mWriteAddr;
	uint8_t mScrollX;
	uint8_t mScrollY;
	uint8_t mMirror;
	uint16_t mLine;
	uint16_t mLineClock;
	uint32_t mFrames;
	uint8_t mWriteMode; // 0 or 1

	uint8_t* mMem;
	uint8_t* mSpriteMem;
	uint8_t* mScreen;

	uint16_t mLastBGNameTableAddr;
	struct Palette* mLastPaletteP;
};

#endif
