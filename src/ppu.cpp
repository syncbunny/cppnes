#include <stdexcept>
#include <cstring>
#include "ppu.h"
#include "events.h"

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

	int xx = x + mScrollX;
	int yy = y + mScrollY;

	int nameTableId = mCR1 & ID_NAME_TABLE_ADDR;
	uint16_t nameTableBase[] = {
		0x2000, 0x2400, 0x2800, 0x2C00
	};
	uint16_t overFlowNTIdMirrorV[] { 1, 0, 3, 2 };
	uint16_t overFlowNTIdMirrorH[] { 2, 3, 0, 1 };

	// calc nametable address
	int u = xx/8;
	int v = yy/8;
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

	this->getColor(bpTable, pat, uu, vv, &mScreen[(y*256+x)*3]);
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
}

void PPU::getColor(uint8_t* base, uint8_t pat, uint8_t u, uint8_t v, uint8_t* rgb) {
	// TODO: use color palette
	u = 7-u;
	uint8_t pat1 = base[pat*16 + v];
	uint8_t pat2 = base[pat*16 + v + 8];
	uint8_t col = 0;
	col |= (pat2 >> u)&0x01; col <<= 1;
	col |= (pat1 >> u)&0x01; col <<= 6;
	rgb[0] = col;
	rgb[1] = col;
	rgb[2] = col;
}
