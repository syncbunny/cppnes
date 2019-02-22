#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include "nesfortest.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "mapper.h"
#include "vmapper.h"

NESForTest::NESForTest()
:NES(){
	mMapper = new VMapper();
	mCPU = new CPU(mMapper);
	mPPU = new PPU();
	mAPU = new APU();

	mDClockCPU = 0;
	mDClockPPU = 0;
	mCartridgeMem = 0;

	mWRAM = new uint8_t[0x0800];
	mMapper->setWRAM(mWRAM);
	mMapper->setPPU(mPPU);
	mMapper->setAPU(mAPU);

	mCROM = new uint8_t[32768];
	mPROM = new uint8_t[8192];
	mMapper->setPROM(mPROM, 31768);
	mMapper->setCROM(mCROM, 8192);
	mMapper->setNo(3);
}

NESForTest::~NESForTest() {
	delete[] mPROM;
	delete[] mCROM;
}

void NESForTest::test() {
}
