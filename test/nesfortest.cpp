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
	mMapper = new Mapper();
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
	cputest_STA_ABS_1();
	cputest_JSR_ABS_1();
	cputest_DEY_1();
	cputest_ROR_ZP_1();
	cputest_ROR_ZP_2();
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

void NESForTest::cputest_STA_ABS_1() {
	// Initialize
	mPROM[0] = 0x8D;
	mPROM[1] = 0x23;
	mPROM[2] = 0x01;
	mTestCPU->testInit();
	mTestCPU->setA(0x56);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_STA_ABS_1: WRAM[0x0123]", mWRAM[0x0123], 0x56);
	check_eq("cputest_STA_ABS_1: PC", mTestCPU->PC(), PROM_BASE_ADDR+3);
}

void NESForTest::cputest_JSR_ABS_1() {
	// Initialize
	mPROM[0] = 0x20; // 0x8000
	mPROM[1] = 0x23; // 0x8001
	mPROM[2] = 0x01; // 0x8002 
	mTestCPU->testInit();
	uint8_t s = mTestCPU->S();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_JSR_ABS_1: PC", mTestCPU->PC(), 0x0123);
	check_eq("cputest_JSR_ABS_1: S", mTestCPU->S(), s-2);
	check_eq("cputest_JSR_ABS_1: Stack", mWRAM[0x0100+s], 0x80);
	check_eq("cputest_JSR_ABS_1: Stack-1", mWRAM[0x0100+s-1], 0x02);
}

void NESForTest::cputest_DEY_1() {
	// Initialize
	mPROM[0] = 0x88; // 0x8000
	mTestCPU->testInit();
	mTestCPU->setY(1);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_DEY_1: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_DEY_1: Y", mTestCPU->Y(), 0x00);
	check_eq("cputest_DEY_1: flag Z", mTestCPU->flagZ(), 1);
	check_eq("cputest_DEY_1: flag N", mTestCPU->flagN(), 0);
}

void NESForTest::cputest_ROR_ZP_1() {
	// Initialize
	mPROM[0] = 0x66;
	mPROM[1] = 0x00;
	mWRAM[0] = 0x22;
	mTestCPU->testInit();
	mTestCPU->setP(0);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROR_ZP_1: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_ROR_ZP_1: WRAM[0]", mWRAM[0], 0x11);
	check_eq("cputest_ROR_ZP_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROR_ZP_1: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_ROR_ZP_1: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_ROR_ZP_2() {
	// Initialize
	mPROM[0] = 0x66;
	mPROM[1] = 0x00;
	mWRAM[0] = 0x11;
	mTestCPU->testInit();
	mTestCPU->setP(0);
	mTestCPU->setFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROR_ZP_2: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_ROR_ZP_2: WRAM[0]", mWRAM[0], 0x88);
	check_eq("cputest_ROR_ZP_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROR_ZP_2: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_ROR_ZP_2: flag C", mTestCPU->flagC(), 1);
}
