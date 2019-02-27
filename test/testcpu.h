#ifndef TEST_CPU_H
#define TEST_CPU_H

#include "cpu.h"

class TestCPU: public CPU {
public:
	TestCPU(class Mapper* mapper);
	virtual ~TestCPU();

public:
	uint8_t A() const {
		return mA;
	}
	uint8_t P() const {
		return mP;
	}
	uint8_t S() const {
		return mS;
	}
	uint16_t PC() const {
		return mPC;
	}
	uint8_t flagZ() const {
		return ((mP&0x02) != 0)? 1:0;
	}
	uint8_t flagN() const {
		return ((mP&0x80) != 0)? 1:0;
	}
	void setA(uint8_t a) {
		mA = a;
	}
public:
	void testInit();
};

#endif

