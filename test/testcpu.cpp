#include "testcpu.h"

TestCPU::TestCPU(Mapper* mapper)
:CPU(mapper) {
}

TestCPU::~TestCPU() {
}

void TestCPU::testInit() {
	mPC = 0x8000;
	mS = 0xFD;
	mClockRemain = 0;
}
