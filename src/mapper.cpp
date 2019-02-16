#include <stdexcept>
#include "mapper.h"
#include "ppu.h"

Mapper::Mapper()
:mNo(0){
}

Mapper::~Mapper() {
}

void Mapper::setWRAM(uint8_t* base) {
	mWRAM = base;
}

void Mapper::setPROM(uint8_t* base, std::size_t size) {
	mPROM = base;
}

void Mapper::setCROM(uint8_t* base, std::size_t size) {
	mCROM = base;
}

void Mapper::setPPU(PPU* ppu) {
	mPPU = ppu;
}

void Mapper::setNo(uint8_t no) {
	mNo = no;
}

void Mapper::write1Byte(uint16_t addr, uint8_t val) {
	if (addr <= 0x07FF) {
		mWRAM[addr] = val;
	} else if (addr == 0x2000) {
		mPPU->setCR1(val);
	} else if (addr == 0x2001) {
		mPPU->setCR2(val);
	} else {
		char msg[1024];
		sprintf(msg, "Mapper::write1Byte: unmapped address(%04x)", addr);
		throw std::runtime_error(msg);
	}
}

void Mapper::write2Bytes(uint16_t addr, uint16_t val) {
	write1Byte(addr, val&0xFF);
	write1Byte(addr+1, val>>8);
}

uint8_t Mapper::read1Byte(uint16_t addr) {
	uint8_t ret = 0;

	if (addr >= 0x8000) {
		ret = mPROM[addr-0x8000];
	}

	return ret;
}

uint16_t Mapper::read2Bytes(uint16_t addr) {
	uint16_t ret = 0;

	if (addr >= 0x8000) {
		ret = mPROM[addr-0x8000];
		ret |= ((uint16_t)mPROM[addr-0x8000 +1] << 8);
	}

	return ret;
}
