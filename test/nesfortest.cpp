#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include "nesfortest.h"
#include "cpu.h"
#include "testcpu.h"
#include "ppu.h"
#include "apu.h"
#include "mapper.h"
#include "vmapper.h"

#define PROM_BASE_ADDR (0x8000)

NESForTest::NESForTest()
:NES(){
	mMapper = new VMapper();
	mCPU = new CPU(mMapper);
	mPPU = new PPU();
	mAPU = new APU();
	mTestCPU = (TestCPU*)mCPU;

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

void NESForTest::check_eq(const char* msg, int val, int expect) {
	if (val != expect) {
		char buf[1024];
		sprintf(buf, "%s: failed. expected=%d, val=%d", msg, expect, val);
		throw std::runtime_error(buf);
	} else {
		printf("%s:OK\n", msg);
	}
}

void NESForTest::test() {
	cputest();
}

void NESForTest::cputest() {
	cputest_LDA_IMM_1();
	cputest_LDA_IMM_2();
}

void NESForTest::cputest_LDA_IMM_1() {
	// Initialize
	mPROM[0] = 0xA9;
	mPROM[1] = 0x00;
	mTestCPU->testInit();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_LDA_IMM_1: A", mTestCPU->A(), 0);
	check_eq("cputest_LDA_IMM_1: PC", mTestCPU->PC(), PROM_BASE_ADDR+2);
	check_eq("cputest_LDA_IMM_1: flag Z", mTestCPU->flagZ(), 1);
	check_eq("cputest_LDA_IMM_1: flag N", mTestCPU->flagN(), 0);
}

void NESForTest::cputest_LDA_IMM_2() {
	// Initialize
	mPROM[0] = 0xA9;
	mPROM[1] = 0x80;
	mTestCPU->testInit();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_LDA_IMM_2: A", mTestCPU->A(), 0x80);
	check_eq("cputest_LDA_IMM_2: PC", mTestCPU->PC(), PROM_BASE_ADDR+2);
	check_eq("cputest_LDA_IMM_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_LDA_IMM_2: flag N", mTestCPU->flagN(), 1);
}

