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
	uint8_t X() const {
		return mX;
	}
	uint8_t Y() const {
		return mY;
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
	uint8_t flagV() const {
		return ((mP&0x40) != 0)? 1:0;
	}
	uint8_t flagC() const {
		return ((mP&0x01) != 0)? 1:0;
	}
	void setA(uint8_t a) {
		mA = a;
	}
	void setX(uint8_t x) {
		mX = x;
	}
	void setY(uint8_t y) {
		mY = y;
	}
	void setP(uint8_t p) {
		mP = p;
	}
	void setFlagC() {
		mP |= 0x01;
	}
	void clearFlagC() {
		mP &= 0xFE;
	}
	void setFlagN() {
		mP |= 0x80;
	}
	void clearFlagN() {
		mP &= 0x7F;
	}
	void setFlagV() {
		mP |= 0x40;
	}
	void clearFlagV() {
		mP &= 0xB0;
	}
	void setFlagZ() {
		mP |= 0x02;
	}
	void clearFlagZ() {
		mP &= 0xFD;
	}
public:
	void testInit();
};

#endif

