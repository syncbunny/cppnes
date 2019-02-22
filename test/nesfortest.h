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
	uint8_t* mPROM;
	uint8_t* mCROM;
};

#endif
