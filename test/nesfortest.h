#ifndef NESFORTEST_H
#define NESFORTEST_H

#include "nes.h"

class NESForTest: public NES {
public:
	NESForTest();
	virtual ~NESForTest();

public:
	void test();

protected:
	void check_eq(const char* msg, int val, int expect);
	void cputest();
	void cputest_LDA_IMM_1();
	void cputest_LDA_IMM_2();
	void cputest_STA_ABS_1();

protected:
	uint8_t* mPROM;
	uint8_t* mCROM;
	class TestCPU* mTestCPU;
};

#endif
