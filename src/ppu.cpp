#include <stdexcept>
#include "ppu.h"

PPU::PPU()
:mMem(0){
	mSR = 0;
	mMem = new uint8_t[0x4000];
}

PPU::~PPU() {
}

void PPU::clock() {
}

void PPU::setScroll(uint8_t val) {
	mScrollVH <<= 8;
	mScrollVH |= val;
}

void PPU::setWriteAddr(uint8_t a) {
	mWriteAddr <<= 8;
	mWriteAddr |= a;
}

void PPU::write(uint8_t val) {
	if (mWriteAddr >= 0x4000) {
                char msg[1024];
		sprintf(msg, "PPU::write: unmapped address(%04x)", mWriteAddr);
		throw std::runtime_error(msg);
	}
	mMem[mWriteAddr] = val;
}
