#include <iostream>
#include <stdexcept>
#include <cstring>
#include <stdio.h>
#include "ppu.h"
#include "events.h"
#include "renderer.h"
#include "core.h"

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

/* Control Register2 &H2001 */
#define BG_COLOR_BLUE           (0x80)
#define BG_COLOR_GREEN          (0x40)
#define BG_COLOR_RED            (0x20)
#define FLAG_ENABLE_SP          (0x10)
#define FLAG_ENABLE_BG          (0x08)
#define FLAG_DRAW_LEFT8_SP      (0x04)
#define FLAG_DRAW_LEFT8_BG      (0x02)
#define FLAG_MONOCHROME_DISPLAY (0x01) // 0: Color Display, 1: Monochrome Display

#define NAME_TABLE_BASE (0x2000)
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

#define SPRITE_ATTRIBUTE_BACK_SPRITE (0x20)
#define SPRITE_ATTRIBUTE_FLIP_H (0x40)
#define SPRITE_ATTRIBUTE_FLIP_V (0x80)
#define STENCIL_BACK_SPRITE (1)
#define STENCIL_BG (2)
#define STENCIL_FRONT_SPRITE (4)

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
	delete[] mStencil;
}

bool PPU::isFrameStart() {
	if (mLine == 0 && mLineClock == 0) {
		return true;
	} else {
		return false;
	}
}

void PPU::clock() {
	if (mLine == 0 && mLineClock == 0) {
		this->frameStart();
	}
	if (mLineClock == 0) {
		CLEAR_SP_HIT();
		this->renderSprite(mLine);
	}

	if (mCR2 & FLAG_ENABLE_BG) {
		this->renderBG(mLineClock, mLine);
	}
	mLineClock++;
	if (mLineClock >= CLOCKS_PAR_LINE) {
		mLineClock -= CLOCKS_PAR_LINE;
		mLine++;
		if (mLine == DRAWABLE_LINES) {
			this->startVR();
		}
		if (mLine >= SCAN_LINES) {
			CLEAR_VBLANK();
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
		mFineX = val&0x07;
		mT.bf1.cx = val >> 3;

		mWriteMode = 1;
	} else {
		mT.bf1.fy = val & 0x07;
		mT.bf1.cy = val >> 3;

		mWriteMode = 0;
	}
}

void PPU::setWriteAddr(uint8_t a) {
	if (mWriteMode == 0) {
		mWriteAddr &= 0x00FF;
		mWriteAddr |= ((uint16_t)a)<<8;

		mT.bf2.b1 = a & 0x3F;
		
		mWriteMode = 1;
	} else {
		mWriteAddr &= 0xFF00;
		mWriteAddr |= a;

		mT.bf2.b2 = a;
		mV = mT;

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

	// Mirroring
	uint16_t addr = mWriteAddr;
	if (mMirror == MIRROR_H) {
		if (addr >= 0x2400 && addr <= 0x27FF) {
			// name table 1 => name table 0
			addr -= 0x0400;
		}
		else if (addr >= 0x2C00 && addr <= 0x2FFF) {
			// name table 3 => name table 2
			addr -= 0x0400;
		}
	}
	if (mMirror == MIRROR_V) {
		if (addr >= 0x2800 && addr <= 0x2FFF) {
			// name table 2,3 => nambe table 0, 1
			addr -= 0x0800;
		}
	}
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		addr = 0x3F00 | (addr&0x001F);
		val &= 0x3F;
	}

	mMem[addr] = val;
	if ((mCR1 & FLAG_ADDR_INC) == 0) {
		mWriteAddr += 1;
	} else {
		mWriteAddr += 32;
	}
}

uint8_t PPU::read() {
	if (mWriteAddr >= 0x4000) {
                char msg[1024];
		sprintf(msg, "PPU::read: unmapped address(%04x)", mWriteAddr);
		throw std::runtime_error(msg);
	}
	uint8_t ret;
	if (mWriteAddr <= 0x3EFF) {
		ret = mReadBuffer;
		mReadBuffer = mMem[mWriteAddr];
	} else {
		ret = mMem[mWriteAddr];
	}
	if ((mCR1 & FLAG_ADDR_INC) == 0) {
		mWriteAddr += 1;
	} else {
		mWriteAddr += 32;
	}

	return ret;
}

void PPU::renderBG(int x, int y) {
	if (x == 257) {
		mV.bf1.n &= 0x2;
		mV.bf1.n |= mT.bf1.n & 0x01;
		mV.bf1.cx = mT.bf1.cx;
	}
	if (y == SCAN_LINES - 1) {
		mV.bf1.cy = mT.bf1.cy;
		mV.bf1.fy = mT.bf1.fy;
		mV.bf1.n &= 0x01;
		mV.bf1.n |= mT.bf1.n & 0x02;
	}

	if (x >= 256 || y >= 240) {
		return;
	}
	if (mStencil[y*256 +x] & STENCIL_FRONT_SPRITE) {
		return;
	}
	if ((x < 8) && ((mCR2 & FLAG_DRAW_LEFT8_BG) == 0)) {
		return;
	}
	int nameTableId = mV.bf1.n;

	int scrollX;
	int scrollY;
	scrollX = mV.bf1.cx;
	scrollX <<=3;
	scrollX += mFineX;
	scrollY = mV.bf1.cy;
	scrollY <<=3;
	scrollY += mV.bf1.fy;
	
	if (nameTableId == 1 || nameTableId == 3) {
		scrollX += 256;
	}
	if (nameTableId == 2 || nameTableId == 3) {
		scrollY += 240;
	}

	int xx = (x + scrollX)%512; // [0 .. 512]
	int yy = (y + scrollY)%480; // [0 .. 512]
	nameTableId = (xx >= 256)? 1:0;
	nameTableId |= (yy >= 240)? 2:0;

	// calc nametable address
	int u = (xx/8)%32; // [0 .. 32]
	int v = (yy/8)%30; // [0 .. 30]

	uint16_t mirrorHNTId[] { 0, 0, 2, 2 };
	uint16_t mirrorVNTId[] { 0, 1, 0, 1 };
	if (mMirror == MIRROR_V) {
		nameTableId = mirrorVNTId[nameTableId];
	} else {
		nameTableId = mirrorHNTId[nameTableId];
	}

	uint16_t nameTableBase[] = {
		0x2000, 0x2400, 0x2800, 0x2C00
	};

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

	uint8_t paletteId;
	if (addr == this->mLastBGNameTableAddr) {
		paletteId = mLastPaletteId;
	} else {
		paletteId = this->getPaletteId(&mMem[nameTableBase[nameTableId]], u, v);
	}
	struct Palette* paletteP = (struct Palette*)&mMem[BG_PALETTE_BASE + paletteId*4];

	if (this->getColor(bpTable, pat, paletteP, uu, vv, &mScreen[(y*256+x)*3])) {
		mStencil[y*256 +x] |= STENCIL_BG;
	}

	this->mLastPaletteId = paletteId;
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
	for (int i = 63; i >= 0; i--) {
		struct Sprite* sp = &sprites[i];
		if (y < sp->y+1 || y >= sp->y+1+8) {
			continue;
		}
		v = y - (sp->y+1);
		if (sp->a & SPRITE_ATTRIBUTE_FLIP_V) {
			v = 7-v;
		}
		uint8_t pat1 = spTable[sp->n*16 + v];
		uint8_t pat2 = spTable[sp->n*16 + v + 8];
		uint8_t attr = sp->a&0x03;
		struct Palette *paletteP = (struct Palette*)&mMem[SPRITE_PALETTE_BASE + attr*4];
		for (int x = 0; x < 256; x++) {
			if ((x < 8) && ((mCR2 & FLAG_DRAW_LEFT8_SP) == 0)) {
				continue;
			}
			if (x < sp->x || x >= sp->x+8) {
				continue;
			}
			if ((i != 0) && (sp->a & SPRITE_ATTRIBUTE_BACK_SPRITE) && (mStencil[y*256+x] & STENCIL_BG)) {
				continue;
			}
			u = x - sp->x;
			uint8_t u2 = u%8; // u2: 0 to 7
			if ((sp->a & SPRITE_ATTRIBUTE_FLIP_H) == 0) {
				u2 = 7-u2;
			}

			uint8_t col = 0;
			col |= (pat2 >> u2)&0x01; col <<= 1;
			col |= (pat1 >> u2)&0x01;
			if (col == 0) {
				continue;
			}
			if (i == 0) {
			//if (i == 0 && (mStencil[y*256+x]&STENCIL_BG)) {
				SET_SP_HIT();
			}
			if ((i == 0) && (sp->a & SPRITE_ATTRIBUTE_BACK_SPRITE) && (mStencil[y*256+x] & STENCIL_BG)) {
				continue;
			}
			col = paletteP->col[col]; // [00 .. 3F]

			mScreen[((y)*256 +x)*3 +0] = colors[col*3 +0];
			mScreen[((y)*256 +x)*3 +1] = colors[col*3 +1];
			mScreen[((y)*256 +x)*3 +2] = colors[col*3 +2];

			if (sp->a & SPRITE_ATTRIBUTE_BACK_SPRITE) {
				mStencil[y*256+x] |= STENCIL_BACK_SPRITE;
			} else {
				mStencil[y*256+x] |= STENCIL_FRONT_SPRITE;
			}
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
	// clear screen and stencil
	struct Palette *paletteP = (struct Palette*)&mMem[SPRITE_PALETTE_BASE];
	uint8_t col = 0;
	if ((mCR2&FLAG_MONOCHROME_DISPLAY) == 0) {
		for (int i = 0; i < 256*240; i++) {
			col = paletteP->col[0];
			mScreen[i*3 +0] = (mCR2&BG_COLOR_RED)?   0xFF:colors[col*3 +0];
			mScreen[i*3 +1] = (mCR2&BG_COLOR_GREEN)? 0xFF:colors[col*3 +1];
			mScreen[i*3 +2] = (mCR2&BG_COLOR_BLUE)?  0xFF:colors[col*3 +2];
		}
	} else {
		for (int i = 0; i < 256*240; i++) {
			col = paletteP->col[0] & 0x30;
			mScreen[i*3 +0] = colors[col*3 +0];
			mScreen[i*3 +1] = colors[col*3 +1];
			mScreen[i*3 +2] = colors[col*3 +2];
		}
	}
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
	if (col == 0) {
		return false;
	}
	col = paletteP->col[col]; // [00 .. 3F]

	rgb[0] = colors[col*3 +0];
	rgb[1] = colors[col*3 +1];
	rgb[2] = colors[col*3 +2];

	return true;
}

/*
 * u: [0 .. 31]
 * v: [0 .. 29]
 */
uint8_t PPU::getPaletteId(uint8_t* base, uint8_t u, uint8_t v) {
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

	return attr;
}

void PPU::capture() {
        char fname[256];
        sprintf(fname, "FRAME_%04d.ppm", mFrames);
        FILE* f = fopen(fname, "w");
        if (f) {
                fprintf(f, "P6\n");
                fprintf(f, "256 240\n");
                fprintf(f, "255\n");
                fwrite(mScreen, 1, 256*240*3, f);
                fclose(f);
        }
}

void PPU::coreDump(Core* c) const {
	struct PPUCore _ppu;

	_ppu.cr1                 = mCR1;
	_ppu.cr2                 = mCR2;
	_ppu.sr                  = mSR;
	_ppu.scrollOffsetTarget  = mScrollOffsetTarget;
	_ppu.writeAddr           = mWriteAddr;
	_ppu.spriteMemAddr       = mSpriteMemAddr;
	_ppu.mirror              = mMirror;
	_ppu.line                = mLine;
	_ppu.lineClock           = mLineClock;
	_ppu.frames              = mFrames;
	_ppu.writeMode           = mWriteMode;
	_ppu.readBuffer          = mReadBuffer;
	_ppu.lastBGNameTableAddr = mLastBGNameTableAddr;
	_ppu.lastPaletteId       = mLastPaletteId;
	_ppu.t1                  = mT.bf2.b1;
	_ppu.t2                  = mT.bf2.b2;
	_ppu.v1                  = mV.bf2.b1;
	_ppu.v2                  = mV.bf2.b2;
	_ppu.fineX               = mFineX;

	memcpy(_ppu.mem, mMem, 0x4000);
	memcpy(_ppu.spriteMem, mSpriteMem, 256);
	memcpy(_ppu.screen, mScreen, 256*240*3);
	memcpy(_ppu.stencil, mStencil, 256*240);

	c->setPPU(_ppu);
}

void PPU::loadCore(Core* c) {
	const struct PPUCore _ppu = c->getPPU();

	this->mCR1 = _ppu.cr1;
	this->mCR2 = _ppu.cr2;
	this->mSR  = _ppu.sr;
	this->mScrollOffsetTarget = _ppu.scrollOffsetTarget;
	this->mWriteAddr          = _ppu.writeAddr;
	this->mSpriteMemAddr      = _ppu.spriteMemAddr;
	this->mMirror             = _ppu.mirror;
	this->mLine               = _ppu.line;
	this->mLineClock          = _ppu.lineClock;
	this->mFrames             = _ppu.frames;
	this->mWriteMode          = _ppu.writeMode;
	this->mReadBuffer         = _ppu.readBuffer;
	this->mLastBGNameTableAddr = _ppu.lastBGNameTableAddr;
	this->mLastPaletteId       = _ppu.lastPaletteId;
	this->mT.bf2.b1            = _ppu.t1;
	this->mT.bf2.b2            = _ppu.t2;
	this->mV.bf2.b1            = _ppu.v1;
	this->mV.bf2.b2            = _ppu.v2;
	this->mFineX               = _ppu.fineX;

	memcpy(this->mMem,       _ppu.mem, 0x4000);
	memcpy(this->mSpriteMem, _ppu.spriteMem, 256);
	memcpy(this->mScreen,    _ppu.screen, 256*240*3);
	memcpy(this->mStencil,   _ppu.stencil, 256*240);
}
