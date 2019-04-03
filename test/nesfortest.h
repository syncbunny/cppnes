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
	void cputest_RESET_1();
	void cputest_LDA_IMM_1();
	void cputest_LDA_IMM_2();
	void cputest_LDA_ABS_1();
	void cputest_STA_ABS_1();
	void cputest_STA_ABS_X_1();
	void cputest_JSR_ABS_1();
	void cputest_INY_1();
	void cputest_DEY_1();
	void cputest_ROR_ZP_1();
	void cputest_ROR_ZP_2();
	void cputest_ASL_A_1();
	void cputest_ASL_A_2();
	void cputest_LSR_A_1();
	void cputest_LSR_A_2();
	void cputest_CMP_IMM_1();
	void cputest_CMP_IMM_2();
	void cputest_CMP_IMM_3();
	void cputest_BPL_1();
	void cputest_BPL_2();
	void cputest_BPL_3();
	void cputest_BNE_1();
	void cputest_BNE_2();
	void cputest_BNE_3();
	void cputest_BEQ_1();
	void cputest_BEQ_2();
	void cputest_BEQ_3();
	void cputest_BMI_1();
	void cputest_BMI_2();
	void cputest_BMI_3();
	void cputest_BCS_1();
	void cputest_BCS_2();
	void cputest_BCS_3();
	void cputest_BCC_1();
	void cputest_BCC_2();
	void cputest_BCC_3();

protected:
	uint8_t* mPROM;
	uint8_t* mCROM;
	class TestCPU* mTestCPU;
};

#endif
