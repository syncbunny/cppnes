#include <cstring>
#include <stdexcept>
#include "mapper.h"
#include "events.h"
#include "ppu.h"
#include "apu.h"
#include "pad.h"

Mapper::Mapper()
:mNo(0), mERAM(0), mPROM(0){
	mPROM = new uint8_t[0x8000]; // 32k bytes
}

Mapper::~Mapper() {
	if (mPROM) {
		delete[] mPROM;
	}
}

void Mapper::setWRAM(uint8_t* base) {
	mWRAM = base;
}

void Mapper::setERAM(uint8_t* base) {
	mERAM = base;
}

void Mapper::setPROM(uint8_t* base, size_t size) {
	if (size == 0x4000) {
		memcpy(&mPROM[0x4000], base, size);
	} else if (size == 0x8000) {
		memcpy(&mPROM[0x0000], base, size);
	} else {
		throw std::runtime_error("invalid PRPOM Size");
	}
}

void Mapper::setCROM(uint8_t* base, size_t size) {
	mCROM = base;
	memcpy(mPPU->getMemory(), mCROM, 0x2000); // 8kbytes
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
	} else if (addr >= 0x0800 && addr <= 0x0FFF) {
		mWRAM[addr-0x0800] = val;
	} else if (addr == 0x2000) {
		mPPU->setCR1(val);
	} else if (addr == 0x2001) {
		mPPU->setCR2(val);
	} else if (addr == 0x2003) {
		mPPU->setSpriteMemAddr(val);
	} else if (addr == 0x2004) {
		mPPU->setSpriteMemVal(val);
	} else if (addr == 0x2005) {
		mPPU->setScroll(val);
	} else if (addr == 0x2006) {
		mPPU->setWriteAddr(val);
	} else if (addr == 0x2007) {
		mPPU->write(val);
	} else if (addr == 0x4000) {
		mAPU->setSW1C1(val);
	} else if (addr == 0x4001) {
		mAPU->setSW1C2(val);
	} else if (addr == 0x4002) {
		mAPU->setSW1FQ1(val);
	} else if (addr == 0x4003) {
		mAPU->setSW1FQ2(val);
	} else if (addr == 0x4004) {
		mAPU->setSW2C1(val);
	} else if (addr == 0x4005) {
		mAPU->setSW2C2(val);
	} else if (addr == 0x4006) {
		mAPU->setSW2FQ1(val);
	} else if (addr == 0x4007) {
		mAPU->setSW2FQ2(val);
	} else if (addr == 0x4008) {
		mAPU->setTWC(val);
	} else if (addr == 0x4009) {
		// unused
	} else if (addr == 0x400A) {
		mAPU->setTWFQ1(val);
	} else if (addr == 0x400B) {
		mAPU->setTWFQ2(val);
	} else if (addr == 0x400C) {
		mAPU->setNZC(val);
	} else if (addr == 0x400D) {
		// unused
	} else if (addr == 0x400E) {
		mAPU->setNZFQ1(val);
	} else if (addr == 0x400F) {
		mAPU->setNZFQ2(val);
	} else if (addr == 0x4010) {
		mAPU->setDMC1(val);
	} else if (addr == 0x4011) {
		mAPU->setDMC2(val);
	} else if (addr == 0x4012) {
		mAPU->setDMC3(val);
	} else if (addr == 0x4013) {
		mAPU->setDMC4(val);
	} else if (addr == 0x4014) {
		this->startDMA(val);
	} else if (addr == 0x4014) {
		this->startDMA(val);
	} else if (addr == 0x4015) {
		mAPU->setChCtrl(val);
	} else if (addr == 0x4016) {
		mPAD->out(val);
	} else if (addr == 0x4017) {
		mAPU->setFrameCounter(val);
	} else if (addr >= 0x6000 && addr <= 0x7FFF) {
		if (mERAM) {
			mERAM[addr-0x6000] = val;
		}
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
	} else if (addr >= 0x0800 && addr <= 0x0FFF) {
		ret = mWRAM[addr-0x0800];
	} else if (addr == 0x2000) {
		ret = mPPU->getCR1();
	} else if (addr == 0x2001) {
		ret = mPPU->getCR1();
	} else if (addr == 0x2002) {
		ret = mPPU->getSR();
	} else if (addr == 0x2007) {
		ret = mPPU->read();
	} else if (addr == 0x4015) {
		ret = mAPU->getChCtrl();
	} else if (addr == 0x4016) {
		ret = mPAD->in1();
	} else if (addr == 0x4017) {
		ret = mPAD->in2();
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
	else if (addr < 0x0800) {
		ret = mWRAM[addr];
		ret |= ((uint16_t)mWRAM[addr+1] << 8);
	}

	return ret;
}

uint16_t Mapper::read2BytesSp(uint16_t addr) {
	uint16_t ret = 0;
	if ((addr&0x00FF) == 0x00FF) {
		if (addr >= 0x8000) {
			ret = mPROM[addr-0x8000];
			addr &= 0xFF00;
			ret |= ((uint16_t)mPROM[addr-0x8000] << 8);
		}
		else if (addr < 0x0800) {
			ret = mWRAM[addr];
			addr &= 0xFF00;
			ret |= ((uint16_t)mWRAM[addr] << 8);
		}
	} else {
		ret = this->read2Bytes(addr);
	}

	return ret;
}

uint16_t Mapper::indirect_x(uint16_t addr, uint8_t x) {
	uint16_t p;
	uint8_t z;

        z = this->read1Byte(addr);
	z += x;

	p = this->read1Byte(z++);
	p |= ((uint16_t)this->read1Byte(z++) << 8);

	return p;
}

uint16_t Mapper::indirect_y(uint16_t addr, uint8_t y) {
	uint16_t p;
	uint8_t z;

        z = this->read1Byte(addr);

	p = this->read1Byte(z++);
	p |= ((uint16_t)this->read1Byte(z++) << 8);
	p += y;

	return p;
}

void Mapper::startDMA(uint8_t val) {
	uint16_t addr = val;
	addr <<= 8;

	uint8_t* dst = mPPU->getSpriteMemAddr();
	std::memcpy(dst, &mWRAM[addr], 256);

	EventQueue& eq = EventQueue::getInstance();
	eq.push(new EventDMA(val));
}
