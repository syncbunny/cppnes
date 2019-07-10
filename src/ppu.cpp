#include <iostream>
#include <stdexcept>
#include <cstring>
#include <stdio.h>
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
#define BG_PALETTE_BASE (0x3F00)
#define SPRITE_PALETTE_BASE (0x3F10)

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

#define SPRITE_ATTRIBUTE_FLIP_H (0x40)
#define SPRITE_ATTRIBUTE_FLIP_V (0x80)
#define STENCIL_BACK_SPRITE (1)
#define STENCIL_BG (2)
#define STENCIL_FRONT_SPRITE (3)

const uint8_t colors[] = {
	/* 00 */ 0x6b, 0x6b, 0x6b, 0x00, 0x10, 0x84, 0x08, 0x00, 0x8c, 0x42, 0x00, 0x7b,
	/* 04 */ 0x63, 0x00, 0x5a, 0x6b, 0x00, 0x10, 0x60, 0x00, 0x00, 0x4f, 0x35, 0x00,
	/* 08 */ 0x31, 0x4e, 0x18, 0x00, 0x5a, 0x21, 0x21, 0x5a, 0x10, 0x08, 0x52, 0x42,
	/* 0c */ 0x00, 0x39, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 10 */ 0xa5, 0xa5, 0xa5, 0x00, 0x42, 0xc6, 0x42, 0x29, 0xce, 0x6b, 0x00, 0xbd,
	/* 14 */ 0x94, 0x29, 0x94, 0x9c, 0x10, 0x42, 0x9c, 0x39, 0x00, 0x84, 0x5e, 0x21,
	/* 18 */ 0x5f, 0x7b, 0x21, 0x2d, 0x8c, 0x29, 0x18, 0x8e, 0x10, 0x2e, 0x86, 0x63,
	/* 1c */ 0x29, 0x73, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 20 */ 0xef, 0xef, 0xef, 0x5a, 0x8c, 0xff, 0x7b, 0x6b, 0xff, 0xa5, 0x5a, 0xff,
	/* 24 */ 0xd6, 0x4a, 0xff, 0xe7, 0x63, 0x9c, 0xde, 0x7b, 0x52, 0xce, 0x9c, 0x29,
	/* 28 */ 0xad, 0xb5, 0x31, 0x7b, 0xce, 0x31, 0x5a, 0xce, 0x52, 0x4a, 0xc6, 0x94,
	/* 2c */ 0x4a, 0xb5, 0xce, 0x52, 0x52, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 30 */ 0xef, 0xef, 0xef, 0xad, 0xc6, 0xff, 0xbd, 0xbd, 0xff, 0xce, 0xb5, 0xff,
	/* 34 */ 0xe7, 0xb5, 0xff, 0xf9, 0xbb, 0xdf, 0xf7, 0xc6, 0xb5, 0xde, 0xc6, 0x9c,
	/* 38 */ 0xd6, 0xd6, 0x94, 0xc6, 0xe7, 0x9c, 0xb5, 0xe7, 0xad, 0xad, 0xe7, 0xc6,
	/* 3c */ 0xad, 0xde, 0xe7, 0xad, 0xad, 0xad, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
	mStencil = new uint8_t[256*240];
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
	if (mLine == 0 && mLineClock == 0) {
		this->frameStart();
	}

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
	if (mStencil[y*256 +x] > STENCIL_BG) {
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
	if (this->getColor(bpTable, pat, paletteP, uu, vv, &mScreen[(y*256+x)*3])) {
		mStencil[y*256 +x] = STENCIL_BG;
	}

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
		if (sp->a & SPRITE_ATTRIBUTE_FLIP_V) {
			v = 7-v;
		}
		uint8_t pat1 = spTable[sp->n*16 + v];
		uint8_t pat2 = spTable[sp->n*16 + v + 8];
		for (int x = 0; x < 256; x++) {
			if (x < sp->x || x >= sp->x+8) {
				continue;
			}
			u = x - sp->x;
			uint8_t u2 = u%8; // u2: 0 to 7
			if ((sp->a & SPRITE_ATTRIBUTE_FLIP_H) == 0) {
				u2 = 7-u2;
			}

			// TODO: use color palette
			uint8_t col = 0;
			col |= (pat2 >> u2)&0x01; col <<= 1;
			col |= (pat1 >> u2)&0x01;;
			if (col == 0) {
				continue;
			}
			if (i == 0) {
				SET_SP_HIT();
			}
			uint8_t attr = sp->a&0x03;
			struct Palette *paletteP = (struct Palette*)&mMem[SPRITE_PALETTE_BASE + attr*4];
			col = paletteP->col[col]; // [00 .. 3F]

			mScreen[((y+1)*256 +x)*3 +0] = colors[col*3 +0];
			mScreen[((y+1)*256 +x)*3 +1] = colors[col*3 +1];
			mScreen[((y+1)*256 +x)*3 +2] = colors[col*3 +2];

			mStencil[(y+1)*256+x] = STENCIL_FRONT_SPRITE;
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
	memset(mStencil, 0, 256*240);
}

void PPU::frameEnd() {
	if (mRenderer) {
		mRenderer->render(mScreen);
	}
}

bool PPU::getColor(uint8_t* base, uint8_t pat, const struct Palette* paletteP, uint8_t uu, uint8_t vv, uint8_t* rgb) {
	uu = 7-uu;
	uint8_t pat1 = base[pat*16 + vv];
	uint8_t pat2 = base[pat*16 + vv + 8];
	uint8_t col = 0;
	col |= (pat2 >> uu)&0x01; col <<= 1;
	col |= (pat1 >> uu)&0x01;
	col = paletteP->col[col]; // [00 .. 3F]

	if (col == 0) {
		return false;
	}

	rgb[0] = colors[col*3 +0];
	rgb[1] = colors[col*3 +1];
	rgb[2] = colors[col*3 +2];

	return true;
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
	int attrUU = (u/2)%2; // [0 .. 1]
	int attrVV = (v/2)%2; // [0 .. 1]
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
	struct Palette *paletteP = (struct Palette*)&mMem[BG_PALETTE_BASE + attr*4];

	return paletteP;
}
