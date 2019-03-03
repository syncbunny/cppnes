#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "nes.h"
#include "cpu.h"
#include "ppu.h"
#include "vppu.h"
#include "apu.h"
#include "pad.h"
#include "vpad.h"
#include "mapper.h"
#include "vmapper.h"

#define FLAG6_V_MIRROR           (0x01)
#define FLAG6_HAS_BATTARY_BACKUP (0x02)
#define FLAG6_HAS_TRAINER        (0x04)
#define FLAG6_HAS_OWN_MIRROR     (0x80)
#define FLAG6_MAPPAER_LOW        (0xF0)

#define FLAG7_VS_UNISYSTEM (0x01)
#define FLAG7_PLAYCHOICE10 (0x02)
#define FLAG7_NES_2_0      (0x0C)
#define FLAG7_MAPPER_HIGH  (0xF0)

NES::NES() {
	mMapper = new VMapper();
	mCPU = new CPU(mMapper);
	//mPPU = new PPU();
	mPPU = new VPPU();
	mAPU = new APU();
	mPAD = new VPAD();

	mDClockCPU = 0;
	mDClockPPU = 0;
	mCartridgeMem = 0;

	mWRAM = new uint8_t[0x0800];
	mMapper->setWRAM(mWRAM);
	mMapper->setPPU(mPPU);
	mMapper->setAPU(mAPU);
	mMapper->setPAD(mPAD);
}

NES::~NES() {
	if (mWRAM) {
		delete[] mWRAM;
	}
	delete mCPU;
	delete mPPU;
	delete mMapper;
}

void NES::powerOn() {
	mCPU->powerOn();
}

bool NES::loadCartridge(const char* path) {
	struct stat st;
	if (stat(path, &st) != 0) {
		fprintf(stderr, "stat(%s) faild(%d)\n", path, errno);
		return false;
	}

	int fd = open(path, 0);
	if (fd == -1) {
		fprintf(stderr, "open faild(%d)\n",errno);
		return false;
	}

	mCartridgeMem = (unsigned char*)mmap(0, st.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (mCartridgeMem == MAP_FAILED) {
		fprintf(stderr, "mmap() faild(%d)\n",errno);
		return false;
	}

	auto checkHeader = [](void* mem) -> bool {
		uint8_t* p = (uint8_t*)mem;
		if (p[0] != 0x4E || p[1] != 0x45 || p[2] != 0x53 || p[3] != 0x1A) {
			return false;
		} else {
			return true;
		}
	};
	if (!checkHeader(mCartridgeMem)) {
		close(fd);
		fprintf(stderr, "cartridge header is invalid.");
		return false;
	}

	uint8_t pROMSize; // in 16k
	uint8_t cROMSize; // in 8k
	pROMSize = mCartridgeMem[4];
	cROMSize = mCartridgeMem[5];

	int offset = 16;
	if (this->cartridgeHasTrainer()) {
		offset += 512;
	}
	mMapper->setPROM(&mCartridgeMem[offset], pROMSize*16*1024);
	mMapper->setCROM(&mCartridgeMem[offset+pROMSize*16*1024], cROMSize*8*1024);
	mMapper->setNo(this->getMapperNo());

	return true;
}

void NES::reset() {
	mCPU->reset();
}

void NES::clock() {
	//       Master          CPU      PPU
	// NTSC: 21477272.72 Hz  Base/12  Base/4

	if (mDClockCPU == 0) {
		mCPU->clock();
		mDClockCPU = 11;
	} else {
		mDClockCPU--;
	}

	if (mDClockPPU == 0) {
		mPPU->clock();
		mDClockPPU = 3;
	} else {
		mDClockPPU--;
	}
}

bool NES::cartridgeHasTrainer() {
	if (mCartridgeMem[5] & FLAG6_HAS_TRAINER) {
		return true;
	} else {
		return false;
	}
}

uint8_t NES::getMapperNo() {
	uint8_t n = 0;

	n = mCartridgeMem[7] & FLAG7_MAPPER_HIGH;
	n |= (mCartridgeMem[6] & FLAG6_MAPPAER_LOW) >> 4;

	return n;
}
