#include <stdexcept>
#include <cstring>
#include "ppu.h"
#include "events.h"
#include "renderer.h"

#define CLOCKS_PAR_LINE (341)
#define DRAWABLE_LINES (240)
#define SCAN_LINES (262)

/* Control Register1 &H2000 */
#define FLAG_NMI_ON_VB (0x80)
#define FLAG_PPU_MASTER_SLAVE (0x40)
#define FLAG_SPRITE_SIZE (0x20) // 0: 8x8, 1: 16x16
#define FLAG_BG_PAT_TABLE_ADDR (0x10) // 0: &H0000, 1: &H1000
#define FLAG_SP_PAT_TABLE_ADDR (0x08) // 0: &H0000, 1: &H1000
#define FLAG_ADDR_INC (0x04) // 0: +1, 1: +32
#define ID_NAME_TABLE_ADDR (0x03) 
#define NAME_TABLE_BASE (0x2800)
//  +-----------+-----------+
//  | 2 ($2800) | 3 ($2C00) |
//  +-----------+-----------+
//  | 0 ($2000) | 1 ($2400) |
//  +-----------+-----------+
#define PALETTE_BASE (0x3F00)

/* Status Register &H2002 */
#define FLAG_VBLANK (0x80)
#define FLAG_SP_HIT (0x40)
#define SCANLINE_SPLITE_OVER (0x20)
#define IFLAG_VBLANK (0x7F)
#define IFLAG_SP_HIT (0xBF)

#define SET_VBLANK() (mSR |= FLAG_VBLANK)
#define CLEAR_VBLANK() (mSR &= IFLAG_VBLANK)
#define SET_SP_HIT() (mSR |= FLAG_SP_HIT)
#define CLEAR_SP_HIT() (mSR &= IFLAG_SP_HIT)

const uint8_t colors[] = {
	/* 00 */ 0x80, 0x80, 0x80, 0x00, 0x3D, 0xA6, 0x00, 0x12, 0xB0, 0x44, 0x00, 0x96,
	/* 04 */ 0xA1, 0x00, 0x5E, 0xC7, 0x00, 0x28, 0xBA, 0x06, 0x00, 0x8C, 0x17, 0x00,
	/* 08 */ 0x5C, 0x2F, 0x00, 0x10, 0x45, 0x00, 0x05, 0x4A, 0x00, 0x00, 0x47, 0x2E,
	/* 0C */ 0x00, 0x41, 0x66, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	/* 10 */ 0xC7, 0xC7, 0xC7, 0x00, 0x77, 0xFF, 0x21, 0x55, 0xFF, 0x82, 0x37, 0xFA,
	/* 14 */ 0xEB, 0x2F, 0xB5, 0xFF, 0x29, 0x50, 0xFF, 0x22, 0x00, 0xD6, 0x32, 0x00,
	/* 18 */ 0xC4, 0x62, 0x00, 0x35, 0x80, 0x00, 0x05, 0x8F, 0x00, 0x00, 0x8A, 0x55,
	/* 1C */ 0x00, 0x99, 0xCC, 0x21, 0x21, 0x21, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	/* 20 */ 0xFF, 0xFF, 0xFF, 0x0F, 0xD7, 0xFF, 0x69, 0xA2, 0xFF, 0xD4, 0x80, 0xFF,
	/* 24 */ 0xFF, 0x45, 0xF3, 0xFF, 0x61, 0x8B, 0xFF, 0x88, 0x33, 0xFF, 0x9C, 0x12,
	/* 28 */ 0xFA, 0xBC, 0x20, 0x9F, 0xE3, 0x0E, 0x2B, 0xF0, 0x35, 0x0C, 0xF0, 0xA4,
	/* 2C */ 0x05, 0xFB, 0xFF, 0x5E, 0x5E, 0x5E, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
	/* 30 */ 0xFF, 0xFF, 0xFF, 0xA6, 0xFC, 0xFF, 0xB3, 0xEC, 0xFF, 0xDA, 0xAB, 0xEB,
	/* 34 */ 0xFF, 0xA8, 0xF9, 0xFF, 0xAB, 0xB3, 0xFF, 0xD2, 0xB0, 0xFF, 0xEF, 0xA6,
	/* 38 */ 0xFF, 0xF7, 0x9C, 0xD7, 0xE8, 0x95, 0xA6, 0xED, 0xAF, 0xA2, 0xF2, 0xDA,
	/* 3C */ 0x99, 0xFF, 0xFC, 0xDD, 0xDD, 0xDD, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
};

struct Sprite {
	uint8_t y;
	uint8_t n;
	uint8_t a;
	uint8_t x;
} __attribute__((packed)) ;

PPU::PPU()
:mMem(0){
	mSR = 0;
	mMem = new uint8_t[0x4000];
	mSpriteMem = new uint8_t[256];
	mScreen = new uint8_t[256*240*3]; // RGB
	mLine = 0;
	mLineClock = 0;
	mFrames = 0;
	mWriteMode = 0;
	mRenderer = 0;
	mWriteAddr = 0;
	mSpriteMemAddr = 0;
}

PPU::~PPU() {
	delete[] mMem;
	delete[] mSpriteMem;
	delete[] mScreen;
}

void PPU::clock() {
	mLineClock++;
	this->renderBG(mLineClock, mLine);
	if (mLineClock >= CLOCKS_PAR_LINE) {
		CLEAR_SP_HIT();
		this->renderSprite(mLine);
		mLineClock -= CLOCKS_PAR_LINE;
		mLine++;
		if (mLine == DRAWABLE_LINES) {
			this->startVR();
		}
		if (mLine >= SCAN_LINES) {
			mLine = 0;
			this->frameEnd();
			mFrames++;
		}
	}
}

void PPU::setSpriteMemAddr(uint8_t a) {
	mSpriteMemAddr = a;
}

void PPU::setSpriteMemVal(uint8_t v) {
	mSpriteMem[mSpriteMemAddr] = v;
	mSpriteMemAddr++;
}

void PPU::setScroll(uint8_t val) {
	if (mWriteMode == 0) {
		mScrollX = val;
		mWriteMode = 1;
	} else {
		mScrollY = val;
		mWriteMode = 0;
	}
}

void PPU::setWriteAddr(uint8_t a) {
	if (mWriteMode == 0) {
		mWriteAddr &= 0x00FF;
		mWriteAddr |= ((uint16_t)a)<<8;
		mWriteMode = 1;
	} else {
		mWriteAddr &= 0xFF00;
		mWriteAddr |= a;
		mWriteMode = 0;
	}
}

uint8_t PPU::getSR() {
	uint8_t sr = mSR;

	mScrollOffsetTarget = 0;
	mWriteMode = 0;
	CLEAR_VBLANK();

	return sr;
}

void PPU::write(uint8_t val) {
	if (mWriteAddr >= 0x4000) {
                char msg[1024];
		sprintf(msg, "PPU::write: unmapped address(%04x)", mWriteAddr);
		throw std::runtime_error(msg);
	}
	mMem[mWriteAddr] = val;
	if ((mCR1 & FLAG_ADDR_INC) == 0) {
		mWriteAddr += 1;
	} else {
		mWriteAddr += 32;
	}
}

void PPU::renderBG(int x, int y) {
	if (x >= 256 || y >= 240) {
		return;
	}

	int xx = (x + mScrollX)%512; // [0 .. 512]
	int yy = (y + mScrollY)%480; // [0 .. 512]

	int nameTableId = mCR1 & ID_NAME_TABLE_ADDR;
	uint16_t nameTableBase[] = {
		0x2000, 0x2400, 0x2800, 0x2C00
	};
	uint16_t overFlowNTIdMirrorV[] { 1, 0, 3, 2 };
	uint16_t overFlowNTIdMirrorH[] { 2, 3, 0, 1 };

	// calc nametable address
	int u = xx/8; // [0 .. 64]
	int v = yy/8; // [0 .. 64]
	if (u >= 32) {
		u -= 32;
		if (mMirror == MIRROR_V) {
			nameTableId = overFlowNTIdMirrorV[nameTableId];
		}
	}
	if (v >= 30) {
		v -= 30;
		if (mMirror == MIRROR_H) {
			nameTableId = overFlowNTIdMirrorH[nameTableId];
		}
	}
	uint16_t addr = nameTableBase[nameTableId] + v*32+u;
	uint8_t pat = mMem[addr];

	uint8_t *bpTable;
	if ((mCR1 & FLAG_BG_PAT_TABLE_ADDR) == 0) {
		bpTable = &mMem[0x0000];
	} else {
		bpTable = &mMem[0x1000];
	}
	int uu = xx%8;
	int vv = yy%8;

	struct Palette *paletteP;
	if (addr == this->mLastBGNameTableAddr) {
		paletteP = mLastPaletteP;
	} else {
		paletteP = this->getPalette(&mMem[nameTableBase[nameTableId]], u, v);
	}
	this->getColor(bpTable, pat, paletteP, uu, vv, &mScreen[(y*256+x)*3]);

	this->mLastPaletteP = paletteP;
	this->mLastBGNameTableAddr = addr;
}

void PPU::renderSprite(int y) {
	if (y >= 239) {
		return;
	}
	struct Sprite* sprites = (struct Sprite*)mSpriteMem;
	uint8_t *spTable;
	if ((mCR1 & FLAG_SP_PAT_TABLE_ADDR) == 0) {
		spTable = &mMem[0x0000];
	} else {
		spTable = &mMem[0x1000];
	}
	int u,v;
	for (int i = 0; i < 64; i++) {
		struct Sprite* sp = &sprites[i];
		if (y+1 < sp->y || y+1 >= sp->y+8 || y+1 >=240) {
			continue;
		}
		v = y+1 - sp->y;
		uint8_t pat1 = spTable[sp->n*16 + v];
		uint8_t pat2 = spTable[sp->n*16 + v + 8];
		for (int x = 0; x < 256; x++) {
			if (x < sp->x || x >= sp->x+8) {
				continue;
			}
			u = x - sp->x;
			uint8_t u2 = u%8; // u2: 0 to 7
			u2 = 7-u2;

			// TODO: use color palette
			uint8_t col = 0;
			col |= (pat2 >> u2)&0x01; col <<= 1;
			col |= (pat1 >> u2)&0x01; col <<= 6;
			if (col != 0 && i == 0) {
				SET_SP_HIT();
			}
			mScreen[((y+1)*256 +x)*3 +0] = col;
			mScreen[((y+1)*256 +x)*3 +1] = col;
			mScreen[((y+1)*256 +x)*3 +2] = col;
		}
	}
}

void PPU::startVR() {
	SET_VBLANK();
	if (mCR1 & FLAG_NMI_ON_VB) {
		EventQueue& eq = EventQueue::getInstance();
		eq.push(new EventNMI());
	}
}

void PPU::frameStart() {
	// TODO: fill with BG color
	memset(mScreen, 0, 256*240*3);
}

void PPU::frameEnd() {
	if (mRenderer) {
		mRenderer->render(mScreen);
	}
}

void PPU::getColor(uint8_t* base, uint8_t pat, const struct Palette* paletteP, uint8_t uu, uint8_t vv, uint8_t* rgb) {
	uu = 7-uu;
	uint8_t pat1 = base[pat*16 + vv];
	uint8_t pat2 = base[pat*16 + vv + 8];
	uint8_t col = 0;
	col |= (pat2 >> uu)&0x01; col <<= 1;
	col |= (pat1 >> uu)&0x01;
	col = paletteP->col[col]; // [00 .. 3F]

	rgb[0] = colors[col*3 +0];
	rgb[1] = colors[col*3 +1];
	rgb[2] = colors[col*3 +2];
}

/*
 * u: [0 .. 31]
 * v: [0 .. 29]
 */
struct Palette* PPU::getPalette(uint8_t* base, uint8_t u, uint8_t v) {
	uint8_t* attrBase = base+0x03C0;
	int attrU = u/4; // [0 .. 8]
	int attrV = v/4; // [0 .. 8]
	uint8_t attr = attrBase[attrV*8 +attrU];
#if 0
	int attrUU = (u%4)/2; // [0 .. 1]
	int attrVV = (v%4)/2; // [0 .. 1]
#else
	int attrUU = (u/2)%2; // [0 .. 1]
	int attrVV = (v/2)%2; // [0 .. 1]
#endif
	int attrUUVV = attrVV*2 + attrUU; // [0 .. 3]
	switch (attrUUVV) {
	// break through
	case 3:
		attr >>= 2;
	case 2:
		attr >>= 2;
	case 1:
		attr >>= 2;
	}
	attr = attr & 0x03; // [0 .. 3]
	struct Palette *paletteP = (struct Palette*)&mMem[PALETTE_BASE + attr*4];

	return paletteP;
}
