#ifndef PPU_H
#define PPU_H

#include <list>
#include <cstdint>

struct Palette {
        uint8_t col[4]; // col[0]: clear
} __attribute__((packed)) ;

class Renderer;
class Core;

struct ScrollBF1 {
	uint8_t u:1; // unused
	uint8_t fy:3;
	uint8_t n:2;
	uint8_t cy:5;
	uint8_t cx:5;
} __attribute__((packed)) ;

struct ScrollBF2 {
	uint8_t b1;
	uint8_t b2;
} __attribute__((packed)) ;

union UScroll {
	struct ScrollBF1 bf1;
	struct ScrollBF2 bf2;
	uint16_t	u16;
};

class PPU {
public:
	enum {
		MIRROR_H, MIRROR_V
	};
public:
	PPU();
	virtual ~PPU();

public:
	virtual void bindRenderer(Renderer* r) {
		mRenderer = r;
	}
	virtual void setMirror(int m) {
		mMirror = m;
	}
	virtual void setCR1(uint8_t v) {
		mCR1 = v;

		mT.bf1.n = v&0x03;
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
	virtual void setSpriteMemAddr(uint8_t a);
	virtual void setSpriteMemVal(uint8_t v);
	virtual void setScroll(uint8_t val);
	virtual void setWriteAddr(uint8_t a);
	virtual void write(uint8_t val);
	virtual uint8_t read();
	virtual void clock();
	virtual void capture();
	virtual void coreDump(Core* c) const;
	virtual void loadCore(Core* c);
	virtual bool isFrameStart();

protected:
	virtual void startVR();
	virtual void frameStart();
	virtual void frameEnd();
	virtual	void renderBG(int x, int y);
	virtual void renderSprite(int y);
	virtual bool getColor(uint8_t* base, uint8_t pat, const struct Palette* paletteP, uint8_t u, uint8_t v, uint8_t* rgb);
	virtual uint8_t getPaletteId(uint8_t* base, uint8_t x, uint8_t y);

protected:
	uint8_t mCR1; // Control Register
	uint8_t mCR2; // Control Register
	uint8_t mSR;  // Status Register
	uint8_t mScrollOffsetTarget;
	uint16_t mWriteAddr;
	uint8_t mSpriteMemAddr;
	uint8_t mMirror;
	uint16_t mLine;
	uint16_t mLineClock;
	uint32_t mFrames;
	uint8_t mWriteMode; // 0 or 1

	union UScroll mV;
	union UScroll mT;
	uint8_t mFineX;

	uint8_t* mMem;
	uint8_t* mSpriteMem;
	uint8_t* mScreen;
	uint8_t* mStencil;

	uint8_t mReadBuffer;
	uint16_t mLastBGNameTableAddr;
	uint8_t mLastPaletteId;

	Renderer* mRenderer;
};

#endif
