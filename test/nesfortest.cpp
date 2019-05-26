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
const uint16_t NMI_VECTOR = 0xFFFA;
const uint16_t RESET_VECTOR = 0xFFFC;
const uint16_t BRK_VECTOR = 0xFFFE;

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
	cputest_RESET_1();
	cputest_LDA_IMM_1();
	cputest_LDA_IMM_2();
	cputest_LDA_ABS_1();
	cputest_STA_ABS_1();
	cputest_STA_ABS_X_1();
	cputest_JSR_ABS_1();
	cputest_ADC_IMM_1();
	cputest_ADC_IMM_2();
	cputest_ADC_IMM_3();
	cputest_SBC_IMM_1();
	cputest_SBC_IMM_2();
	cputest_SBC_IMM_3();
	cputest_INY_1();
	cputest_DEY_1();
	cputest_ROR_ZP_1();
	cputest_ROR_ZP_2();
	cputest_ROL_A_1();
	cputest_ROL_A_2();
	cputest_ROL_A_3();
	cputest_ROR_A_1();
	cputest_ROR_A_2();
	cputest_ROR_A_3();
	cputest_ASL_A_1();
	cputest_ASL_A_2();
	cputest_LSR_A_1();
	cputest_LSR_A_2();
	cputest_CMP_IMM_1();
	cputest_CMP_IMM_2();
	cputest_CMP_IMM_3();
	cputest_BPL_1();
	cputest_BPL_2();
	cputest_BPL_3();
	cputest_BNE_1();
	cputest_BNE_2();
	cputest_BNE_3();
	cputest_BEQ_1();
	cputest_BEQ_2();
	cputest_BEQ_3();
	cputest_BMI_1();
	cputest_BMI_2();
	cputest_BMI_3();
	cputest_BCS_1();
	cputest_BCS_2();
	cputest_BCS_3();
	cputest_BCC_1();
	cputest_BCC_2();
	cputest_BCC_3();
}

void NESForTest::cputest_RESET_1() {
	// Initialize
	mPROM[RESET_VECTOR - PROM_BASE_ADDR +0] = 0x11;
	mPROM[RESET_VECTOR - PROM_BASE_ADDR +1] = 0x80;
	mTestCPU->testInit();
	mTestCPU->reset();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_RESET_1: PC", mTestCPU->PC(), 0x8011);
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

void NESForTest::cputest_LDA_ABS_1() {
	// Initialize
	mPROM[0] = 0xAD;
	mPROM[1] = 0x80;
	mPROM[2] = 0x01;
	mTestCPU->testInit();
	mWRAM[0x0180] = 0x12;

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_LDA_ABS_1: A", mTestCPU->A(), 0x12);
	check_eq("cputest_LDA_ABS_1: PC", mTestCPU->PC(), PROM_BASE_ADDR+3);
	check_eq("cputest_LDA_ABS_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_LDA_ABS_1: flag N", mTestCPU->flagN(), 0);
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

void NESForTest::cputest_STA_ABS_X_1() {
	// Initialize
	mPROM[0] = 0x9D;
	mPROM[1] = 0x23;
	mPROM[2] = 0x01;
	mTestCPU->testInit();
	mTestCPU->setA(0x56);
	mTestCPU->setX(0x81);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_STA_ABS_X_1: WRAM[0x01A4]", mWRAM[0x01A4], 0x56);
	check_eq("cputest_STA_ABS_X_1: PC", mTestCPU->PC(), PROM_BASE_ADDR+3);
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

void NESForTest::cputest_ADC_IMM_1() {
	// Initialize
	mPROM[0] = 0x69;
	mPROM[1] = 0x0F;
	mTestCPU->testInit();
	mTestCPU->setA(0xF0);
	mTestCPU->clearFlagC();
	mTestCPU->setFlagV();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ADC_IMM_1: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_ADC_IMM_1: A", mTestCPU->A(), 0xFF);
	check_eq("cputest_ADC_IMM_1: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_ADC_IMM_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ADC_IMM_1: flag C", mTestCPU->flagC(), 0);
	check_eq("cputest_ADC_IMM_1: flag V", mTestCPU->flagV(), 0);
}

void NESForTest::cputest_ADC_IMM_2() {
	// Initialize
	mPROM[0] = 0x69;
	mPROM[1] = 0x0F;
	mTestCPU->testInit();
	mTestCPU->setA(0xF0);
	mTestCPU->setFlagC();
	mTestCPU->setFlagV();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ADC_IMM_2: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_ADC_IMM_2: A", mTestCPU->A(), 0x00);
	check_eq("cputest_ADC_IMM_2: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_ADC_IMM_2: flag Z", mTestCPU->flagZ(), 1);
	check_eq("cputest_ADC_IMM_2: flag C", mTestCPU->flagC(), 1);
	check_eq("cputest_ADC_IMM_2: flag V", mTestCPU->flagV(), 0);
}

void NESForTest::cputest_ADC_IMM_3() {
	// Initialize
	mPROM[0] = 0x69;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->setA(0x7F);
	mTestCPU->clearFlagC();
	mTestCPU->clearFlagV();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ADC_IMM_3: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_ADC_IMM_3: A", mTestCPU->A(), 0x80);
	check_eq("cputest_ADC_IMM_3: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_ADC_IMM_3: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ADC_IMM_3: flag C", mTestCPU->flagC(), 0);
	check_eq("cputest_ADC_IMM_3: flag V", mTestCPU->flagV(), 1);
}

void NESForTest::cputest_SBC_IMM_1() {
	// Initialize
	mPROM[0] = 0xE9;
	mPROM[1] = 0x0F;
	mTestCPU->testInit();
	mTestCPU->setA(0x0F);
	mTestCPU->setFlagC();
	mTestCPU->setFlagV();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_SBC_IMM_1: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_SBC_IMM_1: A", mTestCPU->A(), 0x00);
	check_eq("cputest_SBC_IMM_1: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_SBC_IMM_1: flag Z", mTestCPU->flagZ(), 1);
	check_eq("cputest_SBC_IMM_1: flag C", mTestCPU->flagC(), 0);
	check_eq("cputest_SBC_IMM_1: flag V", mTestCPU->flagV(), 0);
}

void NESForTest::cputest_SBC_IMM_2() {
	// Initialize
	mPROM[0] = 0xE9;
	mPROM[1] = 0x0F;
	mTestCPU->testInit();
	mTestCPU->setA(0x0F);
	mTestCPU->clearFlagC();
	mTestCPU->setFlagV();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_SBC_IMM_2: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_SBC_IMM_2: A", mTestCPU->A(), 0xFF);
	check_eq("cputest_SBC_IMM_2: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_SBC_IMM_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_SBC_IMM_2: flag C", mTestCPU->flagC(), 1);
	check_eq("cputest_SBC_IMM_2: flag V", mTestCPU->flagV(), 0);
}

void NESForTest::cputest_SBC_IMM_3() {
	// Initialize
	mPROM[0] = 0xE9;
	mPROM[1] = 0x00;
	mTestCPU->testInit();
	mTestCPU->setA(0x80);
	mTestCPU->clearFlagC();
	mTestCPU->setFlagV();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_SBC_IMM_3: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_SBC_IMM_3: A", mTestCPU->A(), 0x7F);
	check_eq("cputest_SBC_IMM_3: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_SBC_IMM_3: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_SBC_IMM_3: flag C", mTestCPU->flagC(), 0);
	check_eq("cputest_SBC_IMM_3: flag V", mTestCPU->flagV(), 1);
}

void NESForTest::cputest_INY_1() {
	// Initialize
	mPROM[0] = 0xC8; // 0x8000
	mTestCPU->testInit();
	mTestCPU->setY(0xFF);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_INY_1: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_INY_1: Y", mTestCPU->Y(), 0x00);
	check_eq("cputest_INY_1: flag Z", mTestCPU->flagZ(), 1);
	check_eq("cputest_INY_1: flag N", mTestCPU->flagN(), 0);
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

void NESForTest::cputest_ROL_A_1() {
	// Initialize
	mPROM[0] = 0x2A;
	mTestCPU->testInit();
	mTestCPU->setA(0x11);
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROL_A_1: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ROL_A_1: A", mTestCPU->A(), 0x22);
	check_eq("cputest_ROL_A_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROL_A_1: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_ROL_A_1: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_ROL_A_2() {
	// Initialize
	mPROM[0] = 0x2A;
	mTestCPU->testInit();
	mTestCPU->setA(0xC8);
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROL_A_2: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ROL_A_2: A", mTestCPU->A(), 0x90);
	check_eq("cputest_ROL_A_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROL_A_2: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_ROL_A_2: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_ROL_A_3() {
	// Initialize
	mPROM[0] = 0x2A;
	mTestCPU->testInit();
	mTestCPU->setA(0x44);
	mTestCPU->setFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROL_A_3: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ROL_A_3: A", mTestCPU->A(), 0x89);
	check_eq("cputest_ROL_A_3: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROL_A_3: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_ROL_A_3: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_ROR_A_1() {
	// Initialize
	mPROM[0] = 0x6A;
	mTestCPU->testInit();
	mTestCPU->setA(0x22);
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROR_A_1: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ROR_A_1: A", mTestCPU->A(), 0x11);
	check_eq("cputest_ROR_A_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROR_A_1: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_ROR_A_1: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_ROR_A_2() {
	// Initialize
	mPROM[0] = 0x6A;
	mTestCPU->testInit();
	mTestCPU->setA(0x11);
	mTestCPU->setFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROR_A_2: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ROR_A_2: A", mTestCPU->A(), 0x88);
	check_eq("cputest_ROR_A_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROR_A_2: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_ROR_A_2: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_ROR_A_3() {
	// Initialize
	mPROM[0] = 0x6A;
	mTestCPU->testInit();
	mTestCPU->setA(0x81);
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ROR_A_3: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ROR_A_3: A", mTestCPU->A(), 0x40);
	check_eq("cputest_ROR_A_3: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ROR_A_3: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_ROR_A_3: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_ASL_A_1() {
	// Initialize
	mPROM[0] = 0x0A;
	mTestCPU->testInit();
	mTestCPU->setFlagC();
	mTestCPU->setA(0x33);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ASL_A_1: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ASL_A_1: A", mTestCPU->A(), 0x66);
	check_eq("cputest_ASL_A_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ASL_A_1: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_ASL_A_2() {
	// Initialize
	mPROM[0] = 0x0A;
	mTestCPU->testInit();
	mTestCPU->setFlagC();
	mTestCPU->setA(0xB3);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_ASL_A_2: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_ASL_A_2: A", mTestCPU->A(), 0x66);
	check_eq("cputest_ASL_A_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_ASL_A_2: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_LSR_A_1() {
	// Initialize
	mPROM[0] = 0x4A;
	mTestCPU->testInit();
	mTestCPU->setFlagC();
	mTestCPU->setA(0x33);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_LSR_A_1: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_LSR_A_1: A", mTestCPU->A(), 0x19);
	check_eq("cputest_LSR_A_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_LSR_A_1: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_LSR_A_2() {
	// Initialize
	mPROM[0] = 0x4A;
	mTestCPU->testInit();
	mTestCPU->setFlagC();
	mTestCPU->setA(0x32);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_LSR_A_2: PC", mTestCPU->PC(), 0x8001);
	check_eq("cputest_LSR_A_2: A", mTestCPU->A(), 0x19);
	check_eq("cputest_LSR_A_2: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_LSR_A_2: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_CMP_IMM_1() {
	// Initialize
	mPROM[0] = 0xC9;
	mPROM[1] = 0x80;
	mTestCPU->testInit();
	mTestCPU->setA(0x81);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_CMP_IMM_1: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_CMP_IMM_1: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_CMP_IMM_1: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_CMP_IMM_1: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_CMP_IMM_2() {
	// Initialize
	mPROM[0] = 0xC9;
	mPROM[1] = 0x81;
	mTestCPU->testInit();
	mTestCPU->setA(0x81);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_CMP_IMM_2: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_CMP_IMM_2: flag N", mTestCPU->flagN(), 0);
	check_eq("cputest_CMP_IMM_2: flag Z", mTestCPU->flagZ(), 1);
	check_eq("cputest_CMP_IMM_2: flag C", mTestCPU->flagC(), 1);
}

void NESForTest::cputest_CMP_IMM_3() {
	// Initialize
	mPROM[0] = 0xC9;
	mPROM[1] = 0x82;
	mTestCPU->testInit();
	mTestCPU->setA(0x81);

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_CMP_IMM_3: PC", mTestCPU->PC(), 0x8002);
	check_eq("cputest_CMP_IMM_3: flag N", mTestCPU->flagN(), 1);
	check_eq("cputest_CMP_IMM_3: flag Z", mTestCPU->flagZ(), 0);
	check_eq("cputest_CMP_IMM_3: flag C", mTestCPU->flagC(), 0);
}

void NESForTest::cputest_BPL_1() {
	// Initialize
	mPROM[0] = 0x10;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->clearFlagN();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BPL_1: PC", mTestCPU->PC(), 0x8003);
}

void NESForTest::cputest_BPL_2() {
	// Initialize
	mPROM[0] = 0x10;
	mPROM[1] = 0xFE;
	mTestCPU->testInit();
	mTestCPU->clearFlagN();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BPL_2: PC", mTestCPU->PC(), 0x8000);
}

void NESForTest::cputest_BPL_3() {
	// Initialize
	mPROM[0] = 0x10;
	mPROM[1] = 0xFF;
	mTestCPU->testInit();
	mTestCPU->setFlagN();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BPL_3: PC", mTestCPU->PC(), 0x8002);
}

void NESForTest::cputest_BNE_1() {
	// Initialize
	mPROM[0] = 0xD0;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->clearFlagZ();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BNE_1: PC", mTestCPU->PC(), 0x8003);
}

void NESForTest::cputest_BNE_2() {
	// Initialize
	mPROM[0] = 0xD0;
	mPROM[1] = 0xFE;
	mTestCPU->testInit();
	mTestCPU->clearFlagZ();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BNE_2: PC", mTestCPU->PC(), 0x8000);
}

void NESForTest::cputest_BNE_3() {
	// Initialize
	mPROM[0] = 0xD0;
	mPROM[1] = 0xFF;
	mTestCPU->testInit();
	mTestCPU->setFlagZ();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BNE_3: PC", mTestCPU->PC(), 0x8002);
}

void NESForTest::cputest_BEQ_1() {
	// Initialize
	mPROM[0] = 0xF0;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->setFlagZ();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BEQ_1: PC", mTestCPU->PC(), 0x8003);
}

void NESForTest::cputest_BEQ_2() {
	// Initialize
	mPROM[0] = 0xF0;
	mPROM[1] = 0xFE;
	mTestCPU->testInit();
	mTestCPU->setFlagZ();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BEQ_2: PC", mTestCPU->PC(), 0x8000);
}

void NESForTest::cputest_BEQ_3() {
	// Initialize
	mPROM[0] = 0xF0;
	mPROM[1] = 0xFF;
	mTestCPU->testInit();
	mTestCPU->clearFlagZ();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BEQ_3: PC", mTestCPU->PC(), 0x8002);
}

void NESForTest::cputest_BMI_1() {
	// Initialize
	mPROM[0] = 0x30;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->setFlagN();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BMI_1: PC", mTestCPU->PC(), 0x8003);
}

void NESForTest::cputest_BMI_2() {
	// Initialize
	mPROM[0] = 0x30;
	mPROM[1] = 0xFE;
	mTestCPU->testInit();
	mTestCPU->setFlagN();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BMI_2: PC", mTestCPU->PC(), 0x8000);
}

void NESForTest::cputest_BMI_3() {
	// Initialize
	mPROM[0] = 0x30;
	mPROM[1] = 0xFF;
	mTestCPU->testInit();
	mTestCPU->clearFlagN();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BMI_3: PC", mTestCPU->PC(), 0x8002);
}

void NESForTest::cputest_BCS_1() {
	// Initialize
	mPROM[0] = 0xB0;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->setFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BCS_1: PC", mTestCPU->PC(), 0x8003);
}

void NESForTest::cputest_BCS_2() {
	// Initialize
	mPROM[0] = 0xB0;
	mPROM[1] = 0xFE;
	mTestCPU->testInit();
	mTestCPU->setFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BCS_2: PC", mTestCPU->PC(), 0x8000);
}

void NESForTest::cputest_BCS_3() {
	// Initialize
	mPROM[0] = 0xB0;
	mPROM[1] = 0xFF;
	mTestCPU->testInit();
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BCS_3: PC", mTestCPU->PC(), 0x8002);
}

void NESForTest::cputest_BCC_1() {
	// Initialize
	mPROM[0] = 0x90;
	mPROM[1] = 0x01;
	mTestCPU->testInit();
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BCC_1: PC", mTestCPU->PC(), 0x8003);
}

void NESForTest::cputest_BCC_2() {
	// Initialize
	mPROM[0] = 0x90;
	mPROM[1] = 0xFE;
	mTestCPU->testInit();
	mTestCPU->clearFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BCC_2: PC", mTestCPU->PC(), 0x8000);
}

void NESForTest::cputest_BCC_3() {
	// Initialize
	mPROM[0] = 0x90;
	mPROM[1] = 0xFF;
	mTestCPU->testInit();
	mTestCPU->setFlagC();

	// Exec
	mTestCPU->clock();

	// Exam
	check_eq("cputest_BCC_3: PC", mTestCPU->PC(), 0x8002);
}
