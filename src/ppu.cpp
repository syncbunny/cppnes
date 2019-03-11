#include <stdexcept>
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
#define FLAG_ADDR_INC // 0: +1, 1: +32
#define ID_NAME_TABLE_ADDR (0x03) 
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

#define SET_VBLANK() (mSR |= FLAG_VBLANK)
#define CLEAR_VBLANK() (mSR &= IFLAG_VBLANK)

PPU::PPU()
:mMem(0){
	mSR = 0;
	mMem = new uint8_t[0x4000];
	mLine = 0;
	mLineClock = 0;
	mFrames = 0;
}

PPU::~PPU() {
}

void PPU::clock() {
	mLineClock++;
	if (mLineClock >= CLOCKS_PAR_LINE) {
		mLineClock -= CLOCKS_PAR_LINE;
		mLine++;
		if (mLine == DRAWABLE_LINES) {
			this->startVR();
		}
		if (mLine >= SCAN_LINES) {
			mLine = 0;
			mFrames++;
		}
	}
}

void PPU::setScroll(uint8_t val) {
	mScrollVH <<= 8;
	mScrollVH |= val;
}

void PPU::setWriteAddr(uint8_t a) {
	mWriteAddr <<= 8;
	mWriteAddr |= a;
}

uint8_t PPU::getSR() {
	uint8_t sr = mSR;

	mScrollOffsetTarget = 0;
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
}

void PPU::startVR() {
	SET_VBLANK();
	if (mCR1 & FLAG_NMI_ON_VB) {
		EventQueue& eq = EventQueue::getInstance();
		eq.push(new EventNMI());
	}
}
