#include <stdexcept>
#include "mapper.h"
#include "ppu.h"
#include "apu.h"
#include "pad.h"

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

void Mapper::setAPU(APU* apu) {
	mAPU = apu;
}

void Mapper::setPAD(PAD* pad) {
	mPAD = pad;
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
	} else if (addr == 0x2005) {
		mPPU->setScroll(val);
	} else if (addr == 0x2006) {
		mPPU->setWriteAddr(val);
	} else if (addr == 0x2007) {
		mPPU->write(val);
	} else if (addr == 0x4010) {
		mAPU->setDMC1(val);
	} else if (addr == 0x4011) {
		mAPU->setDMC2(val);
	} else if (addr == 0x4015) {
		mAPU->setChCtrl(val);
	} else if (addr == 0x4017) {
		mAPU->setFrameCounter(val);
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

void Mapper::push2Bytes(uint16_t addr, uint16_t val) {
	write1Byte(addr--, val>>8);
	write1Byte(addr--, val&0xFF);
}

uint16_t Mapper::pop2Bytes(uint16_t addr) {
	uint16_t ret;

	ret = read1Byte(++addr);
	ret |= ((uint16_t)read1Byte(++addr)) << 8;

	return ret;
}

uint8_t Mapper::read1Byte(uint16_t addr) {
	uint8_t ret = 0;

	if (addr <= 0x07FF) {
		ret = mWRAM[addr];
	} else if (addr == 0x2000) {
		ret = mPPU->getCR1();
	} else if (addr == 0x2001) {
		ret = mPPU->getCR1();
	} else if (addr == 0x2002) {
		ret = mPPU->getSR();
	} else if (addr >= 0x8000) {
		ret = mPROM[addr-0x8000];
	} else {
		char msg[1024];
		sprintf(msg, "Mapper::readByte: unmapped address(%04x)", addr);
		throw std::runtime_error(msg);
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
